/* qthrift thrift network interface
 * Copyright (c) 2016 6WIND,
 *
 * This file is part of GNU Quagga.
 *
 * GNU Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */
#include <zebra.h>
#include "thread.h"
#include "sockunion.h"
#include "sockopt.h"
#include "memory.h"
#include "log.h"
#include "if.h"
#include "prefix.h"
#include "command.h"
#include "privs.h"
#include "linklist.h"
#include "network.h"
#include "filter.h"
#include <glib-object.h>
#include "qthriftd/qthrift_thrift_wrapper.h"
#include "qthriftd/qthriftd.h"
#include "qthriftd/qthrift_debug.h"
#include "qthriftd/qthrift_network.h"
#include "qthriftd/bgp_configurator.h"
#include "qthriftd/bgp_updater.h"
#include "qthriftd/qthrift_bgp_configurator.h"
#include "qthriftd/qthrift_bgp_updater.h"
#include "qthriftd/qthrift_vpnservice.h"

/* qthrift listening socket. */
struct qthrift_listener
{
  /* opaque pointer to qthrift structure */
  void *qthrift;
  struct thread *thread;
};

/* Update BGP socket send buffer size */
#define QTHRIFT_SOCKET_SNDBUF_SIZE    65536

static void
qthrift_update_sock_send_buffer_size (int fd)
{
  int size = QTHRIFT_SOCKET_SNDBUF_SIZE;
  int optval;
  socklen_t optlen = sizeof(optval);

  if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &optval, &optlen) < 0)
    {
      zlog_err("getsockopt of SO_SNDBUF failed %s\n", safe_strerror(errno));
      return;
    }
  if (optval < size)
    {
      if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0)
        {
          zlog_err("Couldn't increase send buffer: %s\n", safe_strerror(errno));
        }
    }
}

/* Accept bgp connection. */
int 
qthrift_accept (struct thread *thread)
{
  struct qthrift_listener *listener = THREAD_ARG(thread);
  struct qthrift_peer *peer, *peer2;
  GError *error = NULL;
  struct qthrift *qthrift = (struct qthrift *)(listener->qthrift);
  ThriftTransport *transport;
  ThriftSocket *socket;
  struct listnode *node, *nnode;

  /* Register accept thread. */
  if( THREAD_FD (thread) < 0)
    {
      zlog_err ("accept_sock is negative value %d", THREAD_FD (thread));
      return -1;
    }
  THREAD_OFF(listener->thread);
  THREAD_READ_ON (tm->master, listener->thread, qthrift_accept, listener, THREAD_FD(thread));

  transport = thrift_server_socket_accept(qthrift->qthrift_vpnservice->bgp_configurator_server_transport,
                                          &error);
  if (transport == NULL)
    {
      zlog_err ("[Error] qthrift server socket accept failed (%s)", safe_strerror (errno));
      return -1;
    }
  peer = qthrift_peer_create_accept(qthrift);
  socket = THRIFT_SOCKET (transport);
  peer->fd = socket->sd;
  qthrift_update_sock_send_buffer_size (socket->sd);
  if(IS_QTHRIFT_DEBUG_NETWORK)
    zlog_info("qthriftd accept : new connection (fd %d)", socket->sd);
  //set_nonblocking (socket->sd);
  peer->peer = XCALLOC(MTYPE_QTHRIFT, \
                       sizeof(struct qthrift_vpnservice_client));
  qthrift_vpnservice_setup_client(peer->peer,
                                  qthrift->qthrift_vpnservice,
                                  transport);
  /* run a thread for reading on accepted socket */
  THREAD_READ_ON (tm->master, peer->t_read, qthrift_read_packet, peer, peer->fd);

  /* close previous thrift connections */
  for (ALL_LIST_ELEMENTS (qthrift->peer, node, nnode, peer2))
    {
      if (peer == peer2)
        continue;

      THREAD_OFF(peer2->t_read);
      list_delete_node (qthrift->peer, node);
      if(peer2->fd)
        {
          if (IS_QTHRIFT_DEBUG)
            zlog_info("qthrift_accept : close connection (fd %d)", peer2->fd);
          qthrift_vpnservice_terminate_client(peer2->peer);
          XFREE(MTYPE_QTHRIFT, peer2->peer);
          peer2->peer = NULL;
          peer2->fd=0;
        }
      XFREE(MTYPE_QTHRIFT, peer2);
    }

  return 0;
}

/* BGP read utility function. */
extern int
qthrift_read_packet (struct thread *thread)
{
  GError *error = NULL;
  struct qthrift_peer *peer = THREAD_ARG(thread);
  gboolean response;
  struct listnode *node;

  response = thrift_dispatch_processor_process (peer->peer->server->processor,\
                                     peer->peer->protocol,\
                                     peer->peer->protocol,\
                                     &error);
  if (error != NULL)
    {
      if(IS_QTHRIFT_DEBUG_NETWORK)
        zlog_info("qthriftd_read_packet: close connection (fd %d)", peer->fd);
      g_clear_error (&error);
      qthrift_vpnservice_terminate_client(peer->peer); 
      XFREE(MTYPE_QTHRIFT, peer->peer);
      peer->peer = NULL;
      peer->fd = 0;
      node = listnode_lookup(peer->qthrift->peer, peer);
      if(node)
        list_delete_node (peer->qthrift->peer, node);
      XFREE(MTYPE_QTHRIFT, peer);
   }
  else 
    {
      peer->t_read = thread_add_read(tm->master, qthrift_read_packet, peer, peer->fd);
    }
  return 0;
}

int
qthrift_server_listen (struct qthrift *qthrift)
{
  struct qthrift_listener *listener;
  GError *error = NULL;
  gboolean ret;

  ret = thrift_server_socket_listen( qthrift->qthrift_vpnservice->bgp_configurator_server_transport, &error);
  if(ret == TRUE)
    {
      ThriftServerSocket *tsocket = \
        THRIFT_SERVER_SOCKET (qthrift->qthrift_vpnservice->bgp_configurator_server_transport);
      listener = XMALLOC (MTYPE_QTHRIFT, sizeof(*listener));
      listener->qthrift = qthrift;
      listener->thread = NULL;
      THREAD_READ_ON (tm->master, listener->thread, qthrift_accept, listener, tsocket->sd);
      listnode_add (tm->listen_sockets, listener);
      return 0;
    }
  zlog_err("qthrift_server_listen : %s (%d)", error?error->message:"", errno);
  return -1;
}

void
qthrift_close (void)
{
  struct listnode *node, *next;
  struct qthrift_listener *listener;
  struct qthrift *qthrift;
  GError *error = NULL;

  for (ALL_LIST_ELEMENTS (tm->listen_sockets, node, next, listener))
    {
      thread_cancel (listener->thread);
      qthrift = listener->qthrift;
      if(qthrift->qthrift_vpnservice->bgp_configurator_server_transport)
        {
          thrift_server_socket_close(qthrift->qthrift_vpnservice->bgp_configurator_server_transport, &error);
          g_object_unref(qthrift->qthrift_vpnservice->bgp_configurator_server_transport);
          qthrift->qthrift_vpnservice->bgp_configurator_server_transport = NULL;
        }
      listnode_delete (tm->listen_sockets, listener);
      XFREE (MTYPE_QTHRIFT, listener);
    }
}

void qthrift_server_socket(struct qthrift *qthrift)
{
#if (!GLIB_CHECK_VERSION (2, 36, 0))
  g_type_init ();
#endif
  qthrift_vpnservice_setup_thrift_bgp_configurator_server(qthrift->qthrift_vpnservice);
  qthrift_bgp_updater_on_start_config_resync_notification ();
  qthrift->qthrift_vpnservice->bgp_update_total++;
  return;
}

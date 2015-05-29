/* bgpd memory type declarations
 *
 * Copyright (C) 2015  David Lamparter
 *
 * This file is part of Quagga.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _QUAGGA_BGP_MEMORY_H
#define _QUAGGA_BGP_MEMORY_H

#include "memory.h"

DECLARE_MGROUP(BGPD)
DECLARE_MTYPE(BGP)
DECLARE_MTYPE(BGP_LISTENER)
DECLARE_MTYPE(BGP_PEER)
DECLARE_MTYPE(BGP_PEER_HOST)
DECLARE_MTYPE(PEER_GROUP)
DECLARE_MTYPE(PEER_DESC)
DECLARE_MTYPE(PEER_PASSWORD)
DECLARE_MTYPE(ATTR)
DECLARE_MTYPE(ATTR_EXTRA)
DECLARE_MTYPE(AS_PATH)
DECLARE_MTYPE(AS_SEG)
DECLARE_MTYPE(AS_SEG_DATA)
DECLARE_MTYPE(AS_STR)

DECLARE_MTYPE(BGP_TABLE)
DECLARE_MTYPE(BGP_NODE)
DECLARE_MTYPE(BGP_ROUTE)
DECLARE_MTYPE(BGP_ROUTE_EXTRA)
DECLARE_MTYPE(BGP_CONN)
DECLARE_MTYPE(BGP_STATIC)
DECLARE_MTYPE(BGP_ADVERTISE_ATTR)
DECLARE_MTYPE(BGP_ADVERTISE)
DECLARE_MTYPE(BGP_SYNCHRONISE)
DECLARE_MTYPE(BGP_ADJ_IN)
DECLARE_MTYPE(BGP_ADJ_OUT)
DECLARE_MTYPE(BGP_MPATH_INFO)

DECLARE_MTYPE(AS_LIST)
DECLARE_MTYPE(AS_FILTER)
DECLARE_MTYPE(AS_FILTER_STR)

DECLARE_MTYPE(COMMUNITY)
DECLARE_MTYPE(COMMUNITY_VAL)
DECLARE_MTYPE(COMMUNITY_STR)

DECLARE_MTYPE(ECOMMUNITY)
DECLARE_MTYPE(ECOMMUNITY_VAL)
DECLARE_MTYPE(ECOMMUNITY_STR)

DECLARE_MTYPE(COMMUNITY_LIST)
DECLARE_MTYPE(COMMUNITY_LIST_NAME)
DECLARE_MTYPE(COMMUNITY_LIST_ENTRY)
DECLARE_MTYPE(COMMUNITY_LIST_CONFIG)
DECLARE_MTYPE(COMMUNITY_LIST_HANDLER)

DECLARE_MTYPE(CLUSTER)
DECLARE_MTYPE(CLUSTER_VAL)

DECLARE_MTYPE(BGP_PROCESS_QUEUE)
DECLARE_MTYPE(BGP_CLEAR_NODE_QUEUE)

DECLARE_MTYPE(TRANSIT)
DECLARE_MTYPE(TRANSIT_VAL)

DECLARE_MTYPE(BGP_DISTANCE)
DECLARE_MTYPE(BGP_NEXTHOP_CACHE)
DECLARE_MTYPE(BGP_CONFED_LIST)
DECLARE_MTYPE(PEER_UPDATE_SOURCE)
DECLARE_MTYPE(BGP_DAMP_INFO)
DECLARE_MTYPE(BGP_DAMP_ARRAY)
DECLARE_MTYPE(BGP_REGEXP)
DECLARE_MTYPE(BGP_AGGREGATE)
DECLARE_MTYPE(BGP_ADDR)
DECLARE_MTYPE(ENCAP_TLV)

#endif /* _QUAGGA_BGP_MEMORY_H */

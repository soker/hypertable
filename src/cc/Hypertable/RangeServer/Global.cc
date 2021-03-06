/** -*- c++ -*-
 * Copyright (C) 2008 Doug Judd (Zvents, Inc.)
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "Common/Compat.h"
#include <cassert>

#include "AsyncComm/ApplicationQueue.h"

#include "Global.h"

using namespace Hypertable;
using namespace Hyperspace;

namespace Hypertable {

  SessionPtr             Global::hyperspace_ptr = 0;
  Filesystem            *Global::dfs = 0;
  Filesystem            *Global::log_dfs = 0;
  MaintenanceQueue      *Global::maintenance_queue = 0;
  RangeServerProtocol   *Global::protocol = 0;
  bool                   Global::verbose = false;
  CommitLog             *Global::user_log = 0;
  CommitLog             *Global::metadata_log = 0;
  CommitLog             *Global::root_log = 0;
  RangeServerMetaLog    *Global::range_log = 0;
  std::string            Global::log_dir = "";
  std::string            Global::location = "";
  uint64_t               Global::range_max_bytes = 0;
  int32_t                Global::access_group_max_files = 0;
  int32_t                Global::access_group_merge_files = 0;
  int32_t                Global::access_group_max_mem = 0;
  ScannerMap             Global::scanner_map;
  FileBlockCache        *Global::block_cache = 0;
  TablePtr               Global::metadata_table_ptr = 0;
  uint64_t               Global::range_metadata_max_bytes = 0;
  MemoryTracker          Global::memory_tracker;
  uint64_t               Global::log_prune_threshold_min = 0;
  uint64_t               Global::log_prune_threshold_max = 0;
  FailureInducer        *Global::failure_inducer = 0;
  double                 Global::bloom_filter_false_positive_rate = 0.01;
}

/** -*- C++ -*-
 * Copyright (C) 2008 Luke Lu (Zvents, Inc.)
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
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

#ifndef HYPERTABLE_LIB_CONFIG_H
#define HYPERTABLE_LIB_CONFIG_H

#include "Common/Config.h"
#include "AsyncComm/Config.h"
#include "Hyperspace/Config.h"
#include "DfsBroker/Lib/Config.h"

namespace Hypertable { namespace Config {

  // init helpers
  void init_master_client_options();
  void init_master_client();
  void init_range_server_client_options();
  void init_range_server_client();
  void init_command_shell_options();

  struct MasterClientPolicy : Policy {
    static void init_options() { init_master_client_options(); }
    static void init() { init_master_client(); }
  };

  struct RangeServerClientPolicy : Policy {
    static void init_options() { init_range_server_client_options(); }
    static void init() { init_range_server_client(); }
  };

  struct CommandShellPolicy : Policy {
    static void init_options() { init_command_shell_options(); }
  };

  typedef Meta::list<DfsClientPolicy, HyperspaceClientPolicy,
          MasterClientPolicy, DefaultCommPolicy> ClientPolicies;

  typedef Join<ClientPolicies>::type DefaultClientPolicy;

}} // namespace Hypertable::Config

#endif // HYPERTABLE_LIB_CONFIG_H

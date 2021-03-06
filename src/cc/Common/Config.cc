/**
 * Copyright (C) 2007 Luke Lu (Zvents, Inc.)
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include "Common/Version.h"
#include "Common/Logger.h"
#include "Common/String.h"
#include "Common/Filesystem.h"
#include "Common/FileUtils.h"
#include "Common/Config.h"
#include "Common/SystemInfo.h"

#include <iostream>
#include <fstream>
#include <errno.h>


namespace Hypertable { namespace Config {

// singletons
RecMutex rec_mutex;
PropertiesPtr properties;
String filename;
bool file_loaded = false;

Desc *cmdline_descp = NULL;
Desc *cmdline_hidden_descp = NULL;
PositionalDesc *cmdline_positional_descp = NULL;
Desc *file_descp = NULL;

int line_length() {
  int n = System::term_info().num_cols;
  return n > 0 ? n : 80;
}

String usage_str(const char *usage) {
  if (!usage)
    usage = "Usage: %s [options]\n\nOptions";

  if (strstr(usage, "%s"))
    return format(usage, System::exe_name.c_str());

  return usage;
}

Desc &cmdline_desc(const char *usage) {
  ScopedRecLock lock(rec_mutex);

  if (!cmdline_descp)
    cmdline_descp = new Desc(usage_str(usage), line_length());

  return *cmdline_descp;
}

Desc &cmdline_hidden_desc() {
  ScopedRecLock lock(rec_mutex);

  if (!cmdline_hidden_descp)
    cmdline_hidden_descp = new Desc();

  return *cmdline_hidden_descp;
}

PositionalDesc &cmdline_positional_desc() {
  ScopedRecLock lock(rec_mutex);

  if (!cmdline_positional_descp)
    cmdline_positional_descp = new PositionalDesc();

  return *cmdline_positional_descp;
}

void cmdline_desc(const Desc &desc) {
  ScopedRecLock lock(rec_mutex);

  if (cmdline_descp)
    delete cmdline_descp;

  cmdline_descp = new Desc(desc);
}

Desc &file_desc(const char *usage) {
  ScopedRecLock lock(rec_mutex);

  if (!file_descp)
    file_descp = new Desc(usage ? usage : "Config Properties", line_length());

  return *file_descp;
}

void file_desc(const Desc &desc) {
  ScopedRecLock lock(rec_mutex);

  if (file_descp)
    delete file_descp;

  file_descp = new Desc(desc);
}

void init_default_options() {
  Path config(System::install_dir);
  config /= "conf/hypertable.cfg";

  cmdline_desc().add_options()
    ("help,h", "Show this help message and exit")
    ("help-file", "Show help message for config file")
    ("version", "Show version information and exit")
    ("verbose,v", boo()->zero_tokens()->default_value(false),
        "Show more verbose output")
    ("debug", boo()->zero_tokens()->default_value(false),
        "Show debug output (shortcut of --logging-level debug)")
    ("quiet", boo()->zero_tokens()->default_value(false), "Negate verbose")
    ("silent", boo()->zero_tokens()->default_value(false),
        "Show as little output as possible")
    ("logging-level,l", str()->default_value("info"),
        "Logging level: debug, info, notice, warn, error, crit, alert, fatal")
    ("config", str()->default_value(config.string()), "Configuration file.\n")
    ("induce-failure", str(), "Arguments for inducing failure")
    ;
  alias("logging-level", "Hypertable.Logging.Level");
  alias("verbose", "Hypertable.Verbose");
  alias("silent", "Hypertable.Silent");

  // pre boost 1.35 doesn't support allow_unregistered, so we have to have the
  // full cfg definition here, which might not be a bad thing.
  file_desc().add_options()
    ("Comm.DispatchDelay", i32()->default_value(0), "[TESTING ONLY] "
        "Delay dispatching of read requests by this number of milliseconds")
    ("Hypertable.Verbose", boo()->default_value(false),
        "Enable verbose output (system wide)")
    ("Hypertable.Silent", boo()->default_value(false),
        "Disable verbose output (system wide)")
    ("Hypertable.Logging.Level", str()->default_value("info"),
        "Set system wide logging level (default: info)")
    ("Hypertable.Request.Timeout", i32()->default_value(20000), "Length of "
        "time, in milliseconds, before timing out requests (system wide)")
    ("Hypertable.MetaLog.SkipErrors", boo()->default_value(false), "Skipping "
        "errors instead of throwing exceptions on metalog errors")
    ("HdfsBroker.Port", i16(),
        "Port number on which to listen (read by HdfsBroker only)")
    ("HdfsBroker.fs.default.name", str(), "Hadoop Filesystem "
        "default name, same as fs.default.name property in Hadoop config "
        "(e.g. hdfs://localhost:9000)")
    ("HdfsBroker.Workers", i32(),
        "Number of HDFS broker worker threads created")
    ("HdfsBroker.Reactors", i32(),
        "Number of HDFS broker communication reactor threads created")
    ("Kfs.Broker.Flush", boo()->default_value(false), "Enable KFS sync/flush")
    ("Kfs.Broker.Workers", i32()->default_value(20), "Number of worker "
        "threads for Kfs broker")
    ("Kfs.Broker.Reactors", i32(), "Number of Kfs broker reactor threads")
    ("Kfs.MetaServer.Name", str(), "Hostname of Kosmos meta server")
    ("Kfs.MetaServer.Port", i16(), "Port number for Kosmos meta server")
    ("DfsBroker.Local.Port", i16()->default_value(38030),
        "Port number on which to listen (read by LocalBroker only)")
    ("DfsBroker.Local.Root", str(), "Root of file and directory "
        "hierarchy for local broker (if relative path, then is relative to "
        "the installation directory)")
    ("DfsBroker.Local.Workers", i32()->default_value(20),
        "Number of local broker worker threads created")
    ("DfsBroker.Local.Reactors", i32(),
        "Number of local broker communication reactor threads created")
    ("DfsBroker.Host", str(),
        "Host on which the DFS broker is running (read by clients only)")
    ("DfsBroker.Port", i16()->default_value(38030),
        "Port number on which DFS broker is listening (read by clients only)")
    ("DfsBroker.Timeout", i32()->default_value(60000), "Length of time, "
        "in milliseconds, to wait before timing out DFS Broker requests. This "
        "takes precedence over Hypertable.Request.Timeout")
    ("Hyperspace.Timeout", i32()->default_value(30000), "Timeout (millisec) "
        "for hyperspace requests (preferred to Hypertable.Request.Timeout")
    ("Hyperspace.Master.Host", str(),
        "Host on which Hyperspace Master is or should be running")
    ("Hyperspace.Master.Port", i16()->default_value(38040),
        "Port number on which Hyperspace Master is or should be listening")
    ("Hyperspace.Master.Workers", i32(),
        "Number of Hyperspace Master worker threads created")
    ("Hyperspace.Master.Reactors", i32(),
        "Number of Hyperspace Master communication reactor threads created")
    ("Hyperspace.Master.Dir", str(), "Root of hyperspace file and directory "
        "heirarchy in local filesystem (if relative path, then is relative to "
        "the installation directory)")
    ("Hyperspace.KeepAlive.Interval", i32()->default_value(10000),
        "Hyperspace Keepalive interval (see Chubby paper)")
    ("Hyperspace.Lease.Interval", i32()->default_value(60000),
        "Hyperspace Lease interval (see Chubby paper)")
    ("Hyperspace.GracePeriod", i32()->default_value(60000),
        "Hyperspace Grace period (see Chubby paper)")
    ("Hypertable.Client.Timeout", i32()->default_value(120000), "Timeout in "
        "(millisec) for Hypertable client API")
    ("Hypertable.Lib.Mutator.FlushDelay", i32(), "Number of "
        "milliseconds to wait prior to flushing scatter buffers (for testing)")
    ("Hypertable.Lib.Mutator.ScatterBuffer.FlushLimit.PerServer",
     i32()->default_value(1*M), "Amount of updates (bytes) accumulated for a "
        "single server to trigger a scatter buffer flush")
    ("Hypertable.Lib.Mutator.ScatterBuffer.FlushLimit.Aggregate",
     i64()->default_value(40*M), "Amount of updates (bytes) accumulated for "
        "all servers to trigger a scatter buffer flush")
    ("Hypertable.LocationCache.MaxEntries", i64()->default_value(1*M),
        "Size of range location cache in number of entries")
    ("Hypertable.Master.Timeout", i32()->default_value(30000), "Timeout "
        "(millisec) for master requests "
        "(prefered to Hypertable.Request.Timeout")
    ("Hypertable.Master.Host", str(),
        "Host on which Hypertable Master is running")
    ("Hypertable.Master.Port", i16()->default_value(38050),
        "Port number on which Hypertable Master is or should be listening")
    ("Hypertable.Master.Workers", i32(),
        "Number of Hypertable Master worker threads created")
    ("Hypertable.Master.Reactors", i32(),
        "Number of Hypertable Master communication reactor threads created")
    ("Hypertable.Master.Gc.Interval", i32()->default_value(300000),
        "Garbage collection interval in milliseconds by Master")
    ("Hypertable.RangeServer.Timeout", i32()->default_value(30000), "Timeout "
        "(millisec) for range server requests "
        "(prefered to Hypertable.Request.Timeout")
    ("Hypertable.RangeServer.Port", i16()->default_value(38060),
        "Port number on which range servers are or should be listening")
    ("Hypertable.RangeServer.AccessGroup.MaxFiles", i32()->default_value(10),
        "Maximum number of cell store files to create before merging")
    ("Hypertable.RangeServer.AccessGroup.MaxMemory", i64()->default_value(50*M),
        "Maximum bytes consumed by an Access Group")
    ("Hypertable.RangeServer.AccessGroup.MergeFiles", i32()->default_value(4),
        "How many files to merge during a merging compaction")
    ("Hypertable.RangeServer.BlockCache.MaxMemory", i64()->default_value(200*M),
        "Bytes to dedicate to the block cache")
    ("Hypertable.RangeServer.Range.MaxBytes", i64()->default_value(200*M),
        "Maximum number of bytes per range before splitting")
    ("Hypertable.RangeServer.Range.MetadataMaxBytes", i64(), "Maximum number "
        "of bytes per METADATA range before splitting (for testing)")
    ("Hypertable.RangeServer.Range.SplitOff", str()->default_value("high"),
        "Portion of range to split off (high or low)")
    ("Hypertable.RangeServer.ClockSkew.Max", i32()->default_value(3*M),
        "Maximum amount of clock skew (microseconds) the system will tolerate")
    ("Hypertable.RangeServer.CommitLog.DfsBroker.Host", str(),
        "Host of DFS Broker to use for Commit Log")
    ("Hypertable.RangeServer.CommitLog.DfsBroker.Port", i16(),
        "Port of DFS Broker to use for Commit Log")
    ("Hypertable.RangeServer.CommitLog.PruneThreshold.Min", i64(),
        "Lower threshold for amount of outstanding commit log before pruning")
    ("Hypertable.RangeServer.CommitLog.PruneThreshold.Max", i64(),
        "Upper threshold for amount of outstanding commit log before pruning")
    ("Hypertable.RangeServer.CommitLog.RollLimit", i64()->default_value(100*M),
        "Roll commit log after this many bytes")
    ("Hypertable.RangeServer.CommitLog.Compressor", str()->default_value("lzo"),
        "Commit log compressor to use (zlib, lzo, quicklz, bmz, none)")
    ("Hypertable.RangeServer.CommitLog.Flush", boo()->default_value(true),
        "Flush the commit log file after each write")
    ("Hypertable.CommitLog.RollLimit", i64()->default_value(100*M),
        "Roll commit log after this many bytes")
    ("Hypertable.CommitLog.Compressor", str()->default_value("lzo"),
        "Commit log compressor to use (zlib, lzo, quicklz, bmz, none)")
    ("Hypertable.CommitLog.Flush", boo()->default_value(true),
        "Flush the commit log file after each write")
    ("Hypertable.CommitLog.SkipErrors", boo()->default_value(false),
        "Skip over any corruption encountered in the commit log")
    ("Hypertable.RangeServer.Scanner.Ttl", i32()->default_value(120000),
        "Number of milliseconds of inactivity before destroying scanners")
    ("Hypertable.RangeServer.Timer.Interval", i32()->default_value(60000),
        "Timer interval in milliseconds (reaping scanners, "
        "purging commit logs, etc.)")
    ("Hypertable.RangeServer.Workers", i32()->default_value(20),
        "Number of Range Server worker threads created")
    ("Hypertable.RangeServer.Reactors", i32(),
        "Number of Range Server communication reactor threads created")
    ("Hypertable.RangeServer.MaintenanceThreads", i32()->default_value(1),
        "Number of maintenance threads")
    ("ThriftBroker.Timeout", i32()->default_value(20*K), "Timeout (ms) "
        "for thrift broker")
    ("ThriftBroker.Port", i16()->default_value(38080), "Port number for "
        "thrift broker")
    ("ThriftBroker.NextLimit", i32()->default_value(100), "Iteration chunk "
        "size (number of cells) for thrift broker")
    ("ThriftBroker.API.Logging", boo()->default_value(false), "Enable or "
        "disable Thrift API logging")
    ;
  alias("Hypertable.RangeServer.CommitLog.RollLimit",
        "Hypertable.CommitLog.RollLimit");
  alias("Hypertable.RangeServer.CommitLog.Compressor",
        "Hypertable.CommitLog.Compressor");
  alias("Hypertable.RangeServer.CommitLog.Flush", "Hypertable.CommitLog.Flush");
  // add config file desc to cmdline hidden desc, so people can override
  // any config values on the command line
  cmdline_hidden_desc().add(file_desc());
}

void parse_args(int argc, char *argv[]) {
  ScopedRecLock lock(rec_mutex);

  HT_TRY("parsing init arguments",
    properties->parse_args(argc, argv, cmdline_desc(), cmdline_hidden_descp,
                           cmdline_positional_descp));
  // some built-in behavior
  if (has("help")) {
    std::cout << cmdline_desc() << std::flush;
    std::exit(0);
  }

  if (has("help-file")) {
    std::cout << file_desc() << std::flush;
    std::exit(0);
  }

  if (has("version")) {
    std::cout << version() << std::endl;
    std::exit(0);
  }

  Path defaultcfg(System::install_dir);
  defaultcfg /= "conf/hypertable.cfg";
  filename = get("config", defaultcfg.string());

  // Only try to parse config file if it exists or not default
  if (FileUtils::exists(filename)) {
    parse_file(filename, file_desc(), properties);
    file_loaded = true;
  }
  else if (filename != defaultcfg)
    HT_THROW(Error::FILE_NOT_FOUND, filename);

  sync_aliases();       // call before use
}

void
parse_file(const String &fname, const Desc &desc, bool allow_unregistered) {
  properties->load(fname, desc, allow_unregistered);
}

void alias(const String &cmdline_opt, const String &file_opt, bool overwrite) {
  properties->alias(cmdline_opt, file_opt, overwrite);
}

void sync_aliases() {
  properties->sync_aliases();
}

void init_default_actions() {
  String loglevel = get_str("logging-level");
  bool verbose = get_bool("verbose");

  if (verbose && get_bool("quiet")) {
    verbose = false;
    properties->set("verbose", false);
  }
  if (get_bool("debug")) {
    loglevel = "debug";
    properties->set("logging-level", loglevel);
  }

  if (loglevel == "info")
    Logger::set_level(Logger::Priority::INFO);
  else if (loglevel == "debug")
    Logger::set_level(Logger::Priority::DEBUG);
  else if (loglevel == "notice")
    Logger::set_level(Logger::Priority::NOTICE);
  else if (loglevel == "warn")
    Logger::set_level(Logger::Priority::WARN);
  else if (loglevel == "error")
    Logger::set_level(Logger::Priority::ERROR);
  else if (loglevel == "crit")
    Logger::set_level(Logger::Priority::CRIT);
  else if (loglevel == "alert")
    Logger::set_level(Logger::Priority::ALERT);
  else if (loglevel == "fatal")
    Logger::set_level(Logger::Priority::FATAL);
  else {
    HT_ERROR_OUT << "unknown logging level: "<< loglevel << HT_END;
    std::exit(1);
  }
  if (verbose) {
    HT_NOTICE_OUT <<"Initializing "<< System::exe_name <<" ("<< version()
                  <<")..." << HT_END;
  }
}

}} // namespace Hypertable::Config

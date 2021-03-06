/** -*- c++ -*-
 * Copyright (C) 2008 Doug Judd (Zvents, Inc.)
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

#include "Common/Compat.h"
#include "Common/Config.h"
#include "Common/DynamicBuffer.h"
#include "Common/FileUtils.h"
#include "Common/InetAddr.h"
#include "Common/System.h"
#include "Common/Usage.h"

#include <iostream>
#include <fstream>

#include "AsyncComm/ConnectionManager.h"

#include "DfsBroker/Lib/Client.h"

#include "Hypertable/Lib/Key.h"
#include "Hypertable/Lib/Schema.h"
#include "Hypertable/Lib/SerializedKey.h"

#include "../CellStoreV0.h"
#include "../FileBlockCache.h"
#include "../Global.h"

#include <cstdlib>

using namespace Hypertable;
using namespace std;

namespace {
  const uint16_t DEFAULT_DFSBROKER_PORT = 38030;
  const char *usage[] = {
    "usage: CellStoreScanner_test",
    "",
    "  This program tests for the proper functioning of the CellStore",
    "  scanner.  It creates a dummy cell store and then repeatedly scans",
    "  it with different ranges",
    (const char *)0
  };
  const char *schema_str =
  "<Schema>\n"
  "  <AccessGroup name=\"default\">\n"
  "    <ColumnFamily id=\"1\">\n"
  "      <Name>tag</Name>\n"
  "    </ColumnFamily>\n"
  "  </AccessGroup>\n"
  "</Schema>";

  const char *words[] = {
    "Libra",
    "prolificity",
    "yonside",
    "testamentarily",
    "hypostasize",
    "scagliola",
    "bandyman",
    "proteinic",
    "paleothermic",
    "heteronereis",
    "undelightfully",
    "Megachilidae",
    "heparinize",
    "salamandrine",
    "stereospondylous",
    "favorableness",
    "aerolitic",
    "ovopyriform",
    "utick",
    "frike",
    "persecutor",
    "fondlesome",
    "Diplodocus",
    "federalism",
    "folkfree",
    "doorlike",
    "paeon",
    "healingly",
    "rugosity",
    "underhorsed",
    "Troic",
    "consummatively",
    "overleap",
    "cuminseed",
    "Hannibal",
    "etiologically",
    "unproperness",
    "Juliana",
    "unsnarl",
    "tarrily",
    "brattie",
    "tettigoniid",
    "garibaldi",
    "spermocarp",
    "palmicolous",
    "Laurencia",
    "basipodite",
    "peculiar",
    "Gulo",
    "heteronymously",
    "undersparred",
    "untoothsome",
    "proprovost",
    "luke",
    "ostracon",
    "Felapton",
    "allotypical",
    "Macrura",
    "verdelho",
    "semicircled",
    "humbug",
    "tenty",
    "Eastertide",
    "epacrid",
    "holoptychiid",
    "unguidedly",
    "Nummulites",
    "fondak",
    "protogine",
    "micromeasurement",
    "mahua",
    "blighting",
    "Guauaenok",
    "bradyacousia",
    "typhomania",
    "ducato",
    "trapezoid",
    "vipresident",
    "coiling",
    "depancreatization",
    "gnathotheca",
    "Balak",
    "micasize",
    "garnetwork",
    "thrack",
    "humoralist",
    "Philistia",
    "tropic",
    "cloysome",
    "oxypropionic",
    "upholstress",
    "therolatry",
    "seaworthiness",
    "nigrification",
    "Calystegia",
    "bulimiform",
    "leptotene",
    "protium",
    "schreinerize",
    "laemodipod",
    "Melastomaceae",
    "nonaluminous",
    "nonsense",
    "chatoyancy",
    "Pomacentrus",
    "kasm",
    "imploringness",
    "syngenesis",
    "infidelly",
    "opianyl",
    "ara",
    "architecturally",
    "unwifelike",
    "nonthinker",
    "Hun",
    "catacorolla",
    "dispulp",
    "formalesque",
    "afterstretch",
    "indicate",
    "turncap",
    "feldspathoid",
    "edea",
    "uralitic",
    "suidian",
    "hydrologist",
    "baar",
    "parapophysial",
    "hoin",
    "cavernoma",
    "ammonion",
    "coly",
    "propitiation",
    "overanalyze",
    "fringent",
    "planetless",
    "ganglia",
    "rehandler",
    "lucidness",
    "anthracyl",
    "Palaeophis",
    "murgavi",
    "Billjim",
    "thinglike",
    "hundredpenny",
    "spherulitic",
    "worserment",
    "phallitis",
    "blaeness",
    "Achernar",
    "Acroceraunian",
    "foreseeable",
    "phytooecology",
    "palaeostracan",
    "enfuddle",
    "Laemodipoda",
    "urolithology",
    "presynaptic",
    "harmonograph",
    "preadequate",
    "bucketful",
    "redoubtableness",
    "submaximal",
    "sociogenesis",
    "epididymodeferential",
    "capsulitis",
    "intercommunicability",
    "understander",
    "chrysomonadine",
    "couseranite",
    "untransposed",
    "argilliferous",
    "Alkoranic",
    "antisynod",
    "undisplaced",
    "indirectness",
    "clashy",
    "uncommitted",
    "glaieul",
    "electromagnetist",
    "underedge",
    "pretenseful",
    "tuberculously",
    "teratoma",
    "documental",
    "Manchester",
    "faciend",
    "cum",
    "lethologica",
    "usitate",
    "Coix",
    "othelcosis",
    "primogenitive",
    "whack",
    "gypsography",
    "earthboard",
    "kempt",
    "backstop",
    "clouding",
    "uncouched",
    "Panayano",
    "brig",
    "utriculosaccular",
    "personalist",
    "medioventral",
    "transcendentalist",
    "acetabuliform",
    "polycyanide",
    "skunkdom",
    "prepreference",
    "delftware",
    "Acanthodini",
    "frostproofing",
    "blackguardry",
    "tetrazane",
    "Olympianism",
    "type",
    "lubrify",
    "B",
    "grazable",
    "Schistosoma",
    "diversifoliate",
    "unconciliable",
    "infra",
    "nonvenous",
    "overbloom",
    "vivisectionally",
    "visualize",
    "postintestinal",
    "catachrestical",
    "tribalist",
    "digynian",
    "outhorror",
    "Tachina",
    "Syriologist",
    "historiette",
    "Amoreuxia",
    "coachmaking",
    "absenter",
    "stibic",
    "subsequence",
    "nonoecumenic",
    "caracolite",
    "Caripuna",
    "Uranicentric",
    "fletcher",
    "acediamine",
    "temerariously",
    "beadlet",
    "bancal",
    "mordication",
    "superdevilish",
    "Polynemus",
    "Interlingua",
    "diaphonia",
    "continentalist",
    "wastement",
    "cubelet",
    "Chordaceae",
    "unsatiableness",
    "prefinal",
    "Amanda",
    "Microsthenes",
    "nonrepetition",
    "corpus",
    "precast",
    "uncrib",
    "delayful",
    "carlings",
    "dilogy",
    "gravimetry",
    "unstoppable",
    "shoreman",
    "speakless",
    "inenarrable",
    "antiholiday",
    "benzolize",
    "inopportunely",
    "decrustation",
    "compositively",
    "Stenopelmatidae",
    "Rhapis",
    "younglet",
    "laeotropism",
    "cannonproof",
    "nonuterine",
    "phyllocyanic",
    "joist",
    "ascidiate",
    "berate",
    "holoquinonoid",
    "analeptical",
    "kynurine",
    "convert",
    "overfavorable",
    "pigflower",
    "suprapubic",
    "Mattapony",
    "citable",
    "urocerid",
    "alternately",
    "Suboscines",
    "dietetically",
    "spiffing",
    "pericarp",
    "placental",
    "circumscriptively",
    "aerophilous",
    "harmonichord",
    "hodder",
    "morphew",
    "marmot",
    "bechirp",
    "superconformity",
    "undomestic",
    "noncensored",
    "hipe",
    "Teutonize",
    "Uragoga",
    "scribing",
    "endodontic",
    "praetaxation",
    "smirkly",
    "Redemptionist",
    "superreform",
    "haploperistomic",
    "posterioric",
    "waeg",
    "chillsome",
    "Cacajao",
    "unpained",
    "conceptacular",
    "unpracticability",
    "melon",
    "expunction",
    "ticketer",
    "Boulangism",
    "laminiplantar",
    "treatyist",
    "smokish",
    "straighten",
    "hypocrystalline",
    "overlength",
    "definitely",
    "Dalmatian",
    "straightforwards",
    "scoldingly",
    "ulsterette",
    "prevolunteer",
    "redespise",
    "polyhybrid",
    "unhanged",
    "habitable",
    "sare",
    "unscramble",
    "milliform",
    "befume",
    "Panhellenium",
    "malacia",
    "omniparous",
    "relenting",
    "rockallite",
    "Royena",
    "Varangi",
    "cytogenous",
    "gnarl",
    "supercoincidence",
    "plurisyllable",
    "skedaddle",
    "reconstructional",
    "interplical",
    "unclassifiable",
    "infructiferous",
    "foldedly",
    "bronchopulmonary",
    "unlawfully",
    "bridgepot",
    "thinkableness",
    "Sudanian",
    "singability",
    "triflorate",
    "slumwise",
    "Aepyceros",
    "nunch",
    "muskroot",
    "eucrasy",
    "Heraclitic",
    "ungirt",
    "tinkerlike",
    "suppletion",
    "marteline",
    "tympanism",
    "octonal",
    "neebor",
    "Texas",
    "semimembranosus",
    "theodolite",
    "zebu",
    "remediable",
    "unsuccinct",
    "aganglionic",
    "labially",
    "Termitidae",
    "irrevocability",
    "Schopenhauerian",
    "expansile",
    "proproctor",
    "theelol",
    "lumen",
    "Mesua",
    "unslept",
    "greyness",
    "palustrine",
    "eventlessness",
    "danalite",
    "bisymmetric",
    "Opisthocomi",
    "outbent",
    "agronome",
    "schizolaenaceous",
    "retake",
    "subdeb",
    "plotted",
    "palsgravine",
    "Gonystylus",
    "stickfast",
    "pretransmit",
    "muffed",
    "statutory",
    "hinoideous",
    "logistical",
    "centralize",
    "stepchild",
    "foreman",
    "oometry",
    "nubigenous",
    "undiminishable",
    "deutoplasm",
    (const char *)0
  };

  size_t display_scan(CellListScannerPtr &scanner_ptr, ostream &out) {
    Key key_comps;
    ByteString bsvalue;
    size_t count = 0;
    while (scanner_ptr->get(key_comps, bsvalue)) {
      out << key_comps << "\n";
      count++;
      scanner_ptr->forward();
    }
    return count;
  }
}


int main(int argc, char **argv) {
  try {
    struct sockaddr_in addr;
    ConnectionManagerPtr conn_mgr;
    DfsBroker::ClientPtr client_ptr;
    CellStorePtr cs;
    std::ofstream out("CellStoreScanner_test.output");
    size_t wordi=0;

    Config::init(argc, argv);

    if (argc != 1)
      Usage::dump_and_exit(usage);

    System::initialize(System::locate_install_dir(argv[0]));
    ReactorFactory::initialize(2);

    InetAddr::initialize(&addr, "localhost", DEFAULT_DFSBROKER_PORT);

    conn_mgr = new ConnectionManager();
    Global::dfs = new DfsBroker::Client(conn_mgr, addr, 15000);

    // force broker client to be destroyed before connection manager
    client_ptr = (DfsBroker::Client *)Global::dfs;

    if (!client_ptr->wait_for_connection(15000)) {
      HT_ERROR("Unable to connect to DFS");
      return 1;
    }

    Global::block_cache = new FileBlockCache(20000000LL);

    String testdir = "/CellStoreScanner_test";
    client_ptr->mkdirs(testdir);

    //String csname = testdir + format("/cs_pid%d", getpid());

    String csname = testdir + "/cs0";

    cs = new CellStoreV0(Global::dfs);

    cs->create(csname.c_str(), 1000, "none", 0, BLOOM_FILTER_DISABLED,
               1.0);

    DynamicBuffer dbuf(64000);
    char rowbuf[256];
    SerializedKey serkey;
    int64_t timestamp = 1;
    std::vector<SerializedKey> serkeyv;
    std::vector<Key> keyv;
    Key key;
    uint8_t valuebuf[128];
    uint8_t *uptr;
    ByteString bsvalue;
    ScanContextPtr scan_ctx_ptr;
    const char *word;
    const char *value = "All work and no play makes jack a dull boy.";

    uptr = valuebuf;
    Serialization::encode_vi32(&uptr, strlen(value));
    strcpy((char *)uptr, value);
    bsvalue.ptr = valuebuf;

    while (dbuf.fill() < 12000) {

      serkey.ptr = dbuf.ptr;
      if ((timestamp % 4) == 0)
        strcpy(rowbuf, "http://www.omega.com/");
      else {
        if (words[wordi] == 0)
          wordi = 0;
        word = words[wordi++];
        sprintf(rowbuf, "http://www.%s.com/", word );
      }

      if (words[wordi] == 0)
        wordi = 0;
      word = words[wordi++];

      create_key_and_append(dbuf, FLAG_INSERT, rowbuf, 1, word, timestamp,
                            timestamp);
      timestamp++;

      serkeyv.push_back(serkey);
    }

    sort(serkeyv.begin(), serkeyv.end());

    keyv.reserve( serkeyv.size() );

    out << "[baseline]\n";
    for (size_t i=0; i<serkeyv.size(); i++) {
      key.load( serkeyv[i] );
      cs->add(key, bsvalue);
      keyv.push_back(key);
      out << key << "\n";
    }

    TableIdentifier table_id;
    cs->finalize(&table_id);

    SchemaPtr schema_ptr = Schema::new_instance(schema_str, strlen(schema_str),
                                                true);
    if (!schema_ptr->is_valid()) {
      HT_ERRORF("Schema Parse Error: %s", schema_ptr->get_error_string());
      exit(1);
    }

    RangeSpec range;
    range.start_row = "";
    range.end_row = Key::END_ROW_MARKER;

    ScanSpecBuilder ssbuilder;
    String column;

    CellListScannerPtr scanner_ptr;

    out << "[individual]\n";
    for (size_t i=0; i<keyv.size(); i++) {
      ssbuilder.clear();
      column = String("tag:") + keyv[i].column_qualifier;
      ssbuilder.add_cell(keyv[i].row, column.c_str());
      scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                     schema_ptr);
      scanner_ptr = cs->create_scanner(scan_ctx_ptr);
      HT_ASSERT(display_scan(scanner_ptr, out) == 1);
    }

    /**
     * Row operations
     */

    out << "[first-block-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Balak.com/", true,
                               "http://www.Boulangism.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Balak.com/", false,
                               "http://www.Boulangism.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Balak.com/", true,
                               "http://www.Boulangism.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Balak.com/", false,
                               "http://www.Boulangism.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.unlawfully.com/", true,
                               "http://www.unscramble.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.unlawfully.com/", false,
                               "http://www.unscramble.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.unlawfully.com/", true,
                               "http://www.unscramble.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.unlawfully.com/", false,
                               "http://www.unscramble.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Philistia.com/", true,
                               "http://www.Texas.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Philistia.com/", false,
                               "http://www.Texas.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Philistia.com/", true,
                               "http://www.Texas.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.Philistia.com/", false,
                               "http://www.Texas.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.antiholiday.com/", true,
                               "http://www.carlings.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.antiholiday.com/", false,
                               "http://www.carlings.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.antiholiday.com/", true,
                               "http://www.carlings.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.antiholiday.com/", false,
                               "http://www.carlings.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.nonvenous.com/", true,
                               "http://www.omega.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.nonvenous.com/", false,
                               "http://www.omega.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.nonvenous.com/", true,
                               "http://www.omega.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.nonvenous.com/", false,
                               "http://www.omega.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-5]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.omega.com/", true,
                               "http://www.oometry.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-6]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.omega.com/", false,
                               "http://www.oometry.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-7]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.omega.com/", true,
                               "http://www.oometry.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-row-scan-8]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.omega.com/", false,
                               "http://www.oometry.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.urolithology.com/", true,
                               "http://www.vipresident.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.urolithology.com/", false,
                               "http://www.vipresident.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.urolithology.com/", true,
                               "http://www.vipresident.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.urolithology.com/", false,
                               "http://www.vipresident.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.utick.com/", true,
                               "http://www.younglet.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.utick.com/", false,
                               "http://www.younglet.com/", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.utick.com/", true,
                               "http://www.younglet.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-row-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_row_interval("http://www.utick.com/", false,
                               "http://www.younglet.com/", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    /**
     * Cell operations
     */

    out << "[first-block-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Balak.com/", "tag:micasize", true,
        "http://www.Boulangism.com/", "tag:laminiplantar", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Balak.com/", "tag:micasize", false,
        "http://www.Boulangism.com/", "tag:laminiplantar", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Balak.com/", "tag:micasize", true,
        "http://www.Boulangism.com/", "tag:laminiplantar", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[first-block-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Balak.com/", "tag:micasize", false,
        "http://www.Boulangism.com/", "tag:laminiplantar", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.unlawfully.com/", "tag:bridgepot",
        true, "http://www.unscramble.com/", "tag:milliform", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.unlawfully.com/", "tag:bridgepot",
        false, "http://www.unscramble.com/", "tag:milliform", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.unlawfully.com/", "tag:bridgepot",
        true, "http://www.unscramble.com/", "tag:milliform", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.unlawfully.com/", "tag:bridgepot",
        false, "http://www.unscramble.com/", "tag:milliform", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Philistia.com/", "tag:tropic", true,
        "http://www.Texas.com/", "tag:semimembranosus", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Philistia.com/", "tag:tropic",
        false, "http://www.Texas.com/", "tag:semimembranosus", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Philistia.com/", "tag:tropic", true,
        "http://www.Texas.com/", "tag:semimembranosus", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[short-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.Philistia.com/", "tag:tropic",
        false, "http://www.Texas.com/", "tag:semimembranosus", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.antiholiday.com/", "tag:benzolize",
        true, "http://www.carlings.com/", "tag:dilogy", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.antiholiday.com/", "tag:benzolize",
        false, "http://www.carlings.com/", "tag:dilogy", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.antiholiday.com/", "tag:benzolize",
        true, "http://www.carlings.com/", "tag:dilogy", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[block-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.antiholiday.com/", "tag:benzolize",
        false, "http://www.carlings.com/", "tag:dilogy", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.nonvenous.com/", "tag:overbloom",
        true, "http://www.omega.com/", "tag:muskroot", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.nonvenous.com/", "tag:overbloom",
        false, "http://www.omega.com/", "tag:muskroot", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.nonvenous.com/", "tag:overbloom",
        true, "http://www.omega.com/", "tag:muskroot", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.nonvenous.com/", "tag:overbloom",
        false, "http://www.omega.com/", "tag:muskroot", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-5]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.omega.com/", "tag:muskroot", true,
        "http://www.oometry.com/", "tag:nubigenous", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-6]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.omega.com/", "tag:muskroot", false,
        "http://www.oometry.com/", "tag:nubigenous", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-7]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.omega.com/", "tag:muskroot", true,
        "http://www.oometry.com/", "tag:nubigenous", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[big-cell-scan-8]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.omega.com/", "tag:muskroot", false,
        "http://www.oometry.com/", "tag:nubigenous", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.urolithology.com/",
        "tag:presynaptic", true, "http://www.vipresident.com/", "tag:coiling",
        true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.urolithology.com/",
        "tag:presynaptic", false, "http://www.vipresident.com/", "tag:coiling",
        true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.urolithology.com/",
        "tag:presynaptic", true, "http://www.vipresident.com/", "tag:coiling",
        false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-short-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.urolithology.com/",
        "tag:presynaptic", false, "http://www.vipresident.com/", "tag:coiling",
        false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-1]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.utick.com/", "tag:frike", true,
        "http://www.younglet.com/", "tag:laeotropism", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-2]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.utick.com/", "tag:frike", false,
        "http://www.younglet.com/", "tag:laeotropism", true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-3]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.utick.com/", "tag:frike", true,
        "http://www.younglet.com/", "tag:laeotropism", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[last-block-cell-scan-4]\n";
    ssbuilder.clear();
    ssbuilder.add_cell_interval("http://www.utick.com/", "tag:frike", false,
        "http://www.younglet.com/", "tag:laeotropism", false);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[cs-range-0]\n";
    cs = new CellStoreV0(Global::dfs);

    cs->open(csname.c_str(), "", "http://www.omega.com/");

    cs->load_index();

    ssbuilder.clear();
    ssbuilder.add_row_interval("", true, Key::END_ROW_MARKER, true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << "[cs-range-1]\n";
    cs = new CellStoreV0(Global::dfs);

    cs->open(csname.c_str(), "http://www.omega.com/", Key::END_ROW_MARKER);

    cs->load_index();

    ssbuilder.clear();
    ssbuilder.add_row_interval("", true, Key::END_ROW_MARKER, true);
    scan_ctx_ptr = new ScanContext(TIMESTAMP_MAX, &(ssbuilder.get()), &range,
                                   schema_ptr);
    scanner_ptr = cs->create_scanner(scan_ctx_ptr);
    display_scan(scanner_ptr, out);

    out << flush;

    String cmd_str = "diff CellStoreScanner_test.output "
                     "CellStoreScanner_test.golden";
    if (system(cmd_str.c_str()) != 0)
      return 1;

    client_ptr->rmdir(testdir);
  }
  catch (Exception &e) {
    HT_ERROR_OUT << e << HT_END;
    return 1;
  }
  catch (...) {
    HT_ERROR_OUT << "unexpected exception caught" << HT_END;
    return 1;
  }
  return 0;
}


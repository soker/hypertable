#!/bin/sh

HT_HOME=${INSTALL_DIR:-"$HOME/hypertable/current"}
HT_SHELL=$HT_HOME/bin/hypertable
SCRIPT_DIR=`dirname $0`
#DATA_SEED=42 # for repeating certain runs

gen_test_data() {
  seed=${DATA_SEED:-$$}
  size=${DATA_SIZE:-"2000000"}
  perl -e 'print "# rowkey\tcolumnkey\tvalue\n"' > data.header
  perl -e 'srand('$seed'); for($i=0; $i<'$size'; ++$i) {
    printf "row%07d\tcolumn%d\tvalue%d\n", $i, int(rand(3))+1, $i
  }' > data.body
  md5sum < data.body > data.md5
}

stop_range_server() {
  # stop any existing range server if necessary
  pidfile=$HT_HOME/run/Hypertable.RangeServer.pid
  if [ -f $pidfile ]; then
    kill -9 `cat $pidfile`
    rm -f $pidfile
    sleep 1

    if $HT_HOME/bin/serverup --silent rangeserver; then
      echo "Can't stop range server, exiting"
      ps -ef | grep Hypertable.RangeServer
      exit 1
    fi
  fi
}

set_tests() {
  for i in $@; do
    eval TEST_$i=1
  done
}

# Runs an individual test
run_test() {
  local TEST_ID=$1
  shift;

  if [ -z "$SKIP_START_SERVERS" ]; then
    $HT_HOME/bin/clean-database.sh
    $HT_HOME/bin/start-all-servers.sh --no-rangeserver local
  fi

  stop_range_server
  $SCRIPT_DIR/rangeserver-launcher.sh $@ > rangeserver.output.$TEST_ID 2>&1 &

  $HT_SHELL --no-prompt < $SCRIPT_DIR/create-test-table.hql
  if [ $? != 0 ] ; then
    echo "Unable to create table 'split-test', exiting ..."
    exit 1
  fi

  $HT_SHELL --no-prompt < $SCRIPT_DIR/load.hql
  if [ $? != 0 ] ; then
    echo "Problem loading table 'split-test', exiting ..."
    exit 1
  fi

  $HT_SHELL -l error --batch < $SCRIPT_DIR/dump-test-table.hql > dbdump.$TEST_ID
  if [ $? != 0 ] ; then
    echo "Problem dumping table 'split-test', exiting ..."
    exit 1
  fi

  md5sum < dbdump.$TEST_ID > dbdump.md5
  diff data.md5 dbdump.md5 > out
  if [ $? != 0 ] ; then
    echo "Test $TEST_ID FAILED." >> report.txt
    cat out >> report.txt
    #exec 1>&-
    #sleep 86400
  else
    echo "Test $TEST_ID PASSED." >> report.txt
  fi
}

rm -f report.txt
gen_test_data

env | grep '^TEST_[0-9]=' || set_tests 0 1 2 3 4 5 6

[ "$TEST_0" ] && run_test 0
[ "$TEST_1" ] && run_test 1 "--induce-failure=split-1:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff high"
[ "$TEST_2" ] && run_test 2 "--induce-failure=split-2:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff high"
[ "$TEST_3" ] && run_test 3 "--induce-failure=split-4:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff high"
[ "$TEST_4" ] && run_test 4 "--induce-failure=split-1:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff low"
[ "$TEST_5" ] && run_test 5 "--induce-failure=split-2:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff low"
[ "$TEST_6" ] && run_test 6 "--induce-failure=split-4:exit:0 \
    --Hypertable.RangeServer.Range.SplitOff low"

echo ""
echo "**** TEST REPORT ****"
echo ""
cat report.txt
grep FAILED report.txt > /dev/null && exit 1
exit 0

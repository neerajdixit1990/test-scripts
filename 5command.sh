for run in {1..205}
do
  # 390 is the magic number where the si & so are 0 for complete duration of benchmark
  time ./simple_benchmark 390
  #docker rm -v $(docker ps -a -q -f status=exited)
  #docker run --rm --volumes-from data cloudsuite/in-memory-analytics /data/ml-latest-small /data/myratings.csv
  #docker run -it --name client --net search_network cloudsuite/web-search:client 172.18.0.2 50 90 60 60
  #docker run --rm --volumes-from data cloudsuite/graph-analytics --driver-memory 16g --executor-memory 16g
  if [ $run -eq 5 ]
  then
    echo "LAST RUN, TURN ON SCRIPT ..."
  fi
done

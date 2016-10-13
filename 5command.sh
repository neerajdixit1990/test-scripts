for run in {1..6}
do
  docker run --rm --volumes-from data cloudsuite/in-memory-analytics /data/ml-latest-small /data/myratings.csv
  if [ $run -eq 5 ]
  then
    echo "LAST RUN, TURN ON SCRIPT ..."
  fi
done

#!/usr/bin/env bash

cat sensor_data-raw.dat \
    | sed -e 's/^[\!#].*$//g' -e 's/Interrupted//g' \
    | awk 'NF {print $1, $2, $3, $4, $5, $6}' \
    > sensor_data-preplot_temp.dat

gnuplot -e "file_in='sensor_data-preplot_temp.dat'; file_out='/var/www/plots/plot-t+rh+co2.png'" \
        gnuplot_settings-t+rh+co2.plg

rm sensor_data-preplot_temp.dat

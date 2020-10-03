reset
set style fill solid
set title 'Performance with cities -CPY -bloom v.s. w/o bloom'
set xlabel 'experiment'
set ylabel 'time(msec)'
set terminal png font " Times_New_Roman,12 "
set output 'out_cities_bloom_CPY.png'

plot [:][:0.004]\
"output_bloom_cities_CPY.txt" using 1:2 with points title "bloom CPY", \
"output_wo_bloom_cities_CPY.txt" using 1:2 with points title "w/o bloom CPY", \

set title 'Performance with cities -REF -bloom v.s. w/o bloom'
set output 'out_cities_bloom_REF.png'
plot [:][:0.004]\
"output_bloom_cities_REF.txt" using 1:2 with points title "bloom REF", \
"output_wo_bloom_cities_REF.txt" using 1:2 with points title "w/o bloom REF", \


set output 'out_rand_bloom_CPY.png'
set title 'Performance with random string -CPY -bloom v.s. w/o bloom'
plot [:][:0.004]\
"output_bloom_rand_CPY.txt" using 1:2 with points title "bloom CPY", \
"output_wo_bloom_rand_CPY.txt" using 1:2 with points title "w/o bloom CPY", \

set output 'out_rand_bloom_REF.png'
set title 'Performance with random string -REF -bloom v.s. w/o bloom'
plot [:][:0.004]\
"output_bloom_rand_REF.txt" using 1:2 with points title "bloom REF", \
"output_wo_bloom_rand_REF.txt" using 1:2 with points title "w/o bloom REF", \


set output 'out_prefix_CPY.png'
set title 'Performance with prefix string -CPY -bloom v.s. w/o bloom'
plot [:][:0.004]\
"output_bloom_prefix_CPY.txt" using 1:2 with points title "bloom CPY", \
"output_wo_bloom_prefix_CPY.txt" using 1:2 with points title "w/o bloom CPY", \

set title 'Performance with prefix string -REF -bloom v.s. w/o bloom'
set output 'out_prefix_REF.png'
plot [:][:0.004]\
"output_bloom_prefix_REF.txt" using 1:2 with points title "bloom REF", \
"output_wo_bloom_prefix_REF.txt" using 1:2 with points title "w/o bloom REF", \


set output 'out_diff_first_CPY.png'
set title 'Performance with different first word -CPY -bloom v.s. w/o bloom'
plot [:][:0.004]\
"output_bloom_first_CPY.txt" using 1:2 with points title "bloom CPY", \
"output_wo_bloom_first_CPY.txt" using 1:2 with points title "w/o bloom CPY", \

set output 'out_diff_first_REF.png'
set title 'Performance with different first word -REF -bloom v.s. w/o bloom'
plot [:][:0.004]\
"output_bloom_first_REF.txt" using 1:2 with points title "bloom REF", \
"output_wo_bloom_first_REF.txt" using 1:2 with points title "w/o bloom REF", \



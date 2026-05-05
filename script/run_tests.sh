#!/bin/bash
PROGRAM="./bin/crypto_benchmark"
DATA_DIR="data"
RESULTS_DIR="results"

# конфиг теста
ALGORITHMS=("xor" "aes")
FILE_SIZES=("1MB" "5MB" "10MB" "25MB" "50MB")
THREADS=4               
CHUNK_SIZE=65536
MEASUREMENTS=30

mkdir -p "$RESULTS_DIR"

echo "Запуск тестов"

# массив для хранения всех результатов
declare -a all_results

# добавляем заголовок для итогового CSV
all_results+=("algorithm,file_size,threads,chunk_size,mean_time_sec")

for algo in "${ALGORITHMS[@]}"; do
    for size in "${FILE_SIZES[@]}"; do
        infile="$DATA_DIR/${size}.bin"
        
        if [ ! -f "$infile" ]; then
            echo "Ошибка: $infile не найден"
            continue
        fi
        
        # последовательная версия
        outfile_seq="$RESULTS_DIR/temp_${algo}_${size}_seq.csv"
        echo -n "Запуск: $algo, $size, sequential"
        
        $PROGRAM -f "$infile" -a "$algo" -m $MEASUREMENTS -o "$outfile_seq" > /dev/null 2>&1
        
        if [ -f "$outfile_seq" ]; then
            mean_time_seq=$(awk -F',' 'NR>1 {sum+=$2; count++} END {if(count>0) printf "%.6f", sum/count; else print "0"}' "$outfile_seq")
            all_results+=("$algo,$size,1,NA,$mean_time_seq")
            printf "%5s %s\n" " - среднее:" "${mean_time_seq} сек"
            rm -f "$outfile_seq"
        else
            echo "Ошибка"
            all_results+=("$algo,$size,1,NA,ERROR")
        fi
        
        # параллельная версия
        outfile_par="$RESULTS_DIR/temp_${algo}_${size}_t${THREADS}.csv"
        echo -n "Запуск: $algo, $size, parallel"
            
        $PROGRAM -f "$infile" -a "$algo" -p -t $THREADS -c $CHUNK_SIZE -m $MEASUREMENTS -o "$outfile_par" > /dev/null 2>&1
            
        if [ -f "$outfile_par" ]; then
            mean_time_par=$(awk -F',' 'NR>1 {sum+=$2; count++} END {if(count>0) printf "%.6f", sum/count; else print "0"}' "$outfile_par")
            all_results+=("$algo,$size,$THREADS,$CHUNK_SIZE,$mean_time_par")
            printf "%5s %s\n" " - среднее:" "${mean_time_par} сек"
            rm -f "$outfile_par"
        else
            echo "Ошибка"
            all_results+=("$algo,$size,$THREADS,NA,ERROR")
        fi
    done
done

# создание итогового файла
SUMMARY_FILE="$RESULTS_DIR/summary.csv"
echo "Итоговый файл: $SUMMARY_FILE"

# запись результатов
printf "%s\n" "${all_results[@]}" > "$SUMMARY_FILE"
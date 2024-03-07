#!/bin/sh
SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=$(dirname "${SCRIPT}")
TEE="tee -a log.txt"
SMALL_NUMBER=5
LARGE_NUMBER=0
cd ${SCRIPT_PATH}/build

cmake -DCMAKE_BUILD_TYPE=Release -DDEBUG_OVERLAPS=0 ..
if [ $? -ne 0 ]; then
    echo "Configuration failed"
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 2
fi

if [ -f log.txt ]; then
    rm log.txt
fi

comment()
{
    #Arguments
    METHOD=$1

    # Comment
    COMMENT=" ${METHOD} #"
    COMMENT_LENGTH=$(echo "${COMMENT}" | wc -m)
    printf '%*s\n' "$COMMENT_LENGTH" | tr ' ' "#" | ${TEE}
    echo "#${COMMENT}" | ${TEE}
    printf '%*s\n' "$COMMENT_LENGTH" | tr ' ' "#" | ${TEE}
}

execute()
{
    #Arguments
    METHOD=$1
    SUBMETHOD=$2
    SEED=$3
    LARGE=$4

    # Compose command
    if [ "${METHOD}" = "greedy" ]; then
        METHOD="${METHOD} --metric ${SUBMETHOD}"
    else
        METHOD="${METHOD} --neighborhood ${SUBMETHOD}"
    fi
    if [ ${LARGE} -gt 0 ]; then
        LARGE="--box_size 50 --item_number 500 --item_size_min 1 --item_size_max 25 --seed ${SEED}"
    else
        LARGE="--box_size 10 --item_number 100 --item_size_min 1 --item_size_max 5 --seed ${SEED}"
    fi
    COMAMND="./optalg_cmd --method ${METHOD} ${LARGE}"

    # Execute
    echo "${COMAMND}" | ${TEE}
    ${COMAMND} | ${TEE}
}

comment "Greedy (area)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "greedy" "area" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "greedy" "area" $i 1; done

comment "Greedy (max size)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "greedy" "max_size" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "greedy" "max_size" $i 1; done

comment "Greedy (min size)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "greedy" "min_size" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "greedy" "min_size" $i 1; done

comment "Neighborhood (geometry)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "neighborhood" "geometry" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "neighborhood" "geometry" $i 1; done

comment "Neighborhood (order)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "neighborhood" "order" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "neighborhood" "order" $i 1; done

comment "Neighborhood (geometry-overlap)"
for i in $(seq 1 ${SMALL_NUMBER}); do execute "neighborhood" "geometry-overlap" $i 0; done
for i in $(seq 1 ${LARGE_NUMBER}); do execute "neighborhood" "geometry-overlap" $i 1; done
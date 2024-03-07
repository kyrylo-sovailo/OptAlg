#!/bin/sh
SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=$(dirname "${SCRIPT}")
TEE="tee -a log.txt"
SMALL_NUMBER=5
LARGE_NUMBER=1
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
    COMMAND="./optalg_cmd --method ${METHOD}"
    if [ "${METHOD}" = "greedy" ]; then
        COMMAND="${COMMAND} --metric ${SUBMETHOD}"
    else
        COMMAND="${COMMAND} --neighborhood ${SUBMETHOD}"
    fi
    if [ ${LARGE} -gt 0 ]; then
        COMMAND="${COMMAND} --box_size 50 --item_number 500 --item_size_min 1 --item_size_max 25"
        if [ "${SUBMETHOD}" = "geometry-overlap" ]; then
            COMMAND="${COMMAND} --desired_iter 500"
        fi
    else
        COMMAND="${COMMAND} --box_size 10 --item_number 100 --item_size_min 1 --item_size_max 5"
        if [ "${SUBMETHOD}" = "geometry-overlap" ]; then
            COMMAND="${COMMAND} --desired_iter 100"
        fi
    fi
    COMMAND="${COMMAND} --seed ${SEED}"
    
    # Execute
    echo "${COMMAND}" | ${TEE}
    ${COMMAND} | ${TEE}
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

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 '{\"value\":\"value to insert\"}'"
fi
curl -s -X POST -d "$1" localhost:2091/insert | jq .

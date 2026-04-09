#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

KAFKA_CONFIG=""

# Check if we are in production environment and set KAFKA_CONFIG accordingly
# or set dev environment variables
if [ "$DAQLITE_PRODUCTION" = "true" ]; then
    KAFKA_CONFIG="-k $DAQLITE_CONFIG/kafka-config-daqlite.json"
else
    DAQLITE_HOME="../build"
    DAQLITE_CONFIG="../configs"
fi

# Set LD_LIBRARY_PATH to include the lib directory for shared libraries
export LD_LIBRARY_PATH="${DAQLITE_HOME}/lib:${LD_LIBRARY_PATH}"

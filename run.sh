#!/bin/bash

if [[ "${@}" == "" ]]; then
	echo    'Usage:'
	echo -e "\t${0} [config] [runs...]"
	exit -1
fi

BASEDIR=$( pwd )

SUMO_LAUNCHD_BASEDIR=./veins/
SUMO_LAUNCHD=./sumo-launchd.py
EXAMPLE=./veins/examples/artery/

CONFIG=${1}
shift
RUNS="${@}"

gitBranch="$( git rev-parse --abbrev-ref HEAD )"

cd ${SUMO_LAUNCHD_BASEDIR}
tmux new-session -d -s ${gitBranch}-${CONFIG}-sumo-launchd "${SUMO_LAUNCHD} -vv -c ${SUMO_HOME}/bin/sumo" && echo 'Started tmux session for sumo-launchd.'
cd ${BASEDIR}

for r in ${RUNS}; do
	cd ${EXAMPLE}
	tmux new-session -d -s ${gitBranch}-${CONFIG}-run-${r} "./run -u Cmdenv -c ${CONFIG} -r ${r}" && echo "Started tmux session for run ${r} of config ${CONFIG}."
	cd ${BASEDIR}
done

tmux ls

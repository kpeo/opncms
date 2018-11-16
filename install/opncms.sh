#!/bin/sh

OPNCMSDIR=/var/www/opncms
OPNCMS=opncms

if [ -e ${OPNCMS}.pid ]; then
	PID=$(cat ${OPNCMS}.pid)
fi

if [ ! -z "$PID" ] && [ -e /proc/${PID} -a /proc/${PID}/exe ]; then
	echo "opnCMS is already running at PID" $PID
else
#	cat /dev/null > ${OPNCMSDIR}/${OPNCMS}.log
	${OPNCMSDIR}/${OPNCMS} -c ${OPNCMSDIR}/config.js & echo $! > ${OPNCMSDIR}/${OPNCMS}.pid
	sleep 1
	chown kpeo:www-data /tmp/${OPNCMS}.sock
	chmod g+w /tmp/${OPNCMS}.sock
fi

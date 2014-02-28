#!/bin/sh
#
# $Id: xastir-fixcfg.sh,v 1.12 2012/11/01 18:57:19 we7u Exp $
#
# Copyright (C) 2003-2012  The Xastir Group
#
# fix up a user's .xastir/config/xastir.cnf to rewrite /usr/local/xastir
# to ${prefix}/share/${name}.
. `dirname $0`/values
CNF=.xastir/config/xastir.cnf
INDEX=.xastir/config/map_index.sys
SELECT=.xastir/config/selected_maps.sys
cd
if [ -f $CNF ]; then
    rm $INDEX
    mv $CNF $CNF.backup
    mv $SELECT $SELECT.backup
    if [ $? -ne 0 ]; then
	echo "$CNF: unable to rename!"
	exit 1
    fi
    sed -e 's:/usr/local/xastir/:${prefix}/share/xastir/:' <$CNF.backup >$CNF
    if [ $? -ne 0 ]; then
	echo "$CNF: sed failed!"
	mv $CNF.backup $CNF
	exit 1
    fi
else
    echo "No $CNF to edit"
fi
echo "Done.  Old config file is in $CNF.backup"
exit 0



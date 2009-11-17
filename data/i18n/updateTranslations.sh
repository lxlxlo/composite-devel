#!/bin/sh

echo "Updating translation (*.ts) files"

cd ../../gui/



if [ "$QTDIR" ]; then
	LUPDATE="$QTDIR/bin/lupdate"
	LRELEASE="$QTDIR/bin/lrelease"
else
	LUPDATE=$(which lupdate)
	LRELEASE=$(which lrelease)
fi;

UI=`find . | grep "\.ui$"`
CPP=`find . | grep "\.cpp$"`
H=`find . | grep "\.h$"`
FILES="$UI $CPP $H"

CMD="$LUPDATE -noobsolete ${FILES} -ts"



$CMD ../data/i18n/composite.it.ts
$CMD ../data/i18n/composite.es.ts
$CMD ../data/i18n/composite.ru.ts
$CMD ../data/i18n/composite.fr.ts
$CMD ../data/i18n/composite.pt_BR.ts
$CMD ../data/i18n/composite.hu_HU.ts
$CMD ../data/i18n/composite.pl.ts
$CMD ../data/i18n/composite.nl.ts
$CMD ../data/i18n/composite.ja.ts
$CMD ../data/i18n/composite.de.ts
$CMD ../data/i18n/composite.sv.ts
$CMD ../data/i18n/composite.hr.ts

echo "Creating *.qm files"
cd ../data/i18n
$LRELEASE *.ts


echo "Stats"
./stats.py

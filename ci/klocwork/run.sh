#!/bin/bash -xe

ART_DIR=result
OUT_DIR=${ART_DIR}/klocwork

OUT_FMT=scriptable

KW_EXE=$HOME/.klocwork/kw.sh
KW_INST_DIR=ci/klocwork/opt

OUT_FILE_EXT=csv

OUT_FILE=issues.${OUT_FILE_EXT}

CHECKER_REPO_SERVER=e2data
CHECKER_REPO_BRANCH=draft
CHECKER_REPO_NAME=eb-denovo-checker
CHECKER_REPO_PATH=/data/e2data/projects/elektrobit/baseos/git

CHECKER_REPO=${CHECKER_REPO_SERVER}:${CHECKER_REPO_PATH}/${CHECKER_REPO_NAME}.git

function kw_install()
{
	if [ ! -f ${KW_EXE} ]
	then
		return 1
	fi

	${KW_EXE} \
	    -a \
	    --klocwork-server klocwork.emlix.com:8080 \
	    --license-server klocwork.emlix.com:27000 \
	    --use-ssl \
	    --debug \
	    -i ${KW_INST_DIR} \
	    ;

	export PATH=$PATH:${KW_INST_DIR}/bin
}

function kw_try_import_profile()
{
	set +e
	git clone \
		${CHECKER_REPO} \
		--branch ${CHECKER_REPO_BRANCH} \
		--single-branch \
		;

	kwcheck import ${CHECKER_REPO_NAME}/analysis_profile.pconf
	set -e

}

function kw_setup()
{
	kwcheck create

	kwcheck import ci/klocwork/taxonomy.tconf
	kw_try_import_profile
}

function kw_analyse()
{
	rm -rf build/klocwork
	mkdir -p build/klocwork
	cmake -B build/klocwork \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_VERBOSE_MAKEFILE=On \
		-DUNIT_TESTS=Off \
		-DENABLE_COVERAGE=Off \
		$BASEDIR
	kwshell make -C build/klocwork

	kwcheck run -F ${OUT_FMT}
}

function kw_store()
{
	mkdir -p ${OUT_DIR}
	kwcheck list -F ${OUT_FMT} --taxonomy emlix --report ${OUT_DIR}/${OUT_FILE}
	sed -i 's@'$PWD/'@@' ${OUT_DIR}/${OUT_FILE}

	install -m 444 \
		.kwps/localconfig/profiles/analysis_profile.pconf \
		${OUT_DIR}

	install -m 444 \
		ci/klocwork/taxonomy.tconf \
		${OUT_DIR}
}

# MAIN =========================================================================
if ! kw_install
then
	echo "WARNING: This stage is expected to run on jenkins"
	exit 0
fi

kw_setup
kw_analyse
kw_store

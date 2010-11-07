#!/bin/bash


cvs_host=stgteam.dp.ua
cvs_user=<user>
cvs_pass=<password>

#arc_name=stg-2.4-`date "+%Y.%m.%d-%H.%M.%S"`.tgz
#arc_name=stg-2.4-`date "+%Y.%m.%d-%H.%M.%S"`.tgz
src_dir=stg-2.4-`date "+%Y.%m.%d-%H.%M.%S"`
arc_name=$src_dir.tar.gz

#mkdir $src_dir

./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stgincludes include $src_dir

./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/common.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/ibpp.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/common_settings.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/conffiles.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/crypto.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/stg_logger.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/stg_locker.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/hostallow.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/pinger.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/dotconfpp.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/ia_auth_c.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/script_executer.lib $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/srvconf.lib $src_dir

./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/Makefile $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stglibs stglibs/Makefile.in $src_dir
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stargazer convertor $src_dir/projects
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stargazer stargazer $src_dir/projects

./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/sgauth sgauth $src_dir/projects
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/sgconf sgconf $src_dir/projects
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/rscriptd rscriptd $src_dir/projects
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stargazer rlm_stg $src_dir/projects

./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stgplugins plugins/store/firebird $src_dir/projects/stargazer
./get_from_cvs $cvs_user $cvs_pass $cvs_host /cvsroot/stg3plugins max_mods/mysql $src_dir/projects/stargazer/plugins/store
mv $src_dir/projects/stargazer/plugins/store/max_mods/mysql $src_dir/projects/stargazer/plugins/store/
rm -rf $src_dir/projects/stargazer/plugins/store/max_mods
rm -rf $src_dir/projects/stargazer/plugins/other/userstat
rm -rf $src_dir/projects/stargazer/plugins/authorization/stress
rm -rf $src_dir/projects/stargazer/plugins/store/db
rm -rf $src_dir/projects/stargazer/plugins/configuration/rpcconfig

rm -f $src_dir/include/lp2_blocks.h
rm -f $src_dir/include/stdstring.h

mkdir -p $src_dir/lib

rm -fr $(find $src_dir/ -name CVS -type d)

rm -fr $src_dir/projects/stargazer/inst/var/stargazer/users/CVS

tar -czf $arc_name $src_dir

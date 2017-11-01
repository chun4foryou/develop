/**
 * @mainpage sn_worker 모듈 메인 파일이
 * @section intro 소개
 * - sn_controller를 통해 받은 룰 파일을 sniper에 덮어 쒸운다
 * @section Program 프로그램명
 * - 프로그램명 : sn_worker
 * @version 1.0
 * @section CREATEINFO 작성정보
 * - 최초 작성자 : 황지윤
 * - 인수 인계자 : 김진권
 * @section MODIFYINFO 수정정보
 * - commit 참조
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <curl/curl.h>
#include <fcntl.h>
#include <errno.h>
//#include <sqlite3.h>
#include <dirent.h>


/**
 * @brief 
 *
 * @param data
 *
 * @return 
 */
int get_upload_file(DATA_t *data)
{

	char dir_path[UPDATE_FILE_PATH_MAX] = {0};
	char file_path[UPDATE_FILE_PATH_MAX] = {0};
	int m_code;
	char minor[2] = {0};

	snprintf(minor, 2, "%c", data->type_code[TYPE_MINOR]);
	m_code = atoi(minor);

	__AGENT_DBGP(__INFO, "Worker get upload file process!! mode[%d]\n", m_code);
	switch(m_code){
		case UCODE_MINOR_PATTERN:
		case UCODE_MINOR_RATELIMIT:
			//Make zip dir
			if (access (LIVEUPDATE_PATH, R_OK)) mkdir (LIVEUPDATE_PATH, 0700);

			if (access (LIVEUPDATE_UPLOAD_PATH, R_OK)) mkdir (LIVEUPDATE_UPLOAD_PATH, 0700);	

			snprintf(dir_path, sizeof(dir_path), "%s/%s", LIVEUPDATE_UPLOAD_PATH, data->update_date);
			if (access (dir_path, R_OK)) mkdir(dir_path, 0700);

			snprintf(file_path, sizeof(file_path), "%s/%s.tar.gz", dir_path, data->update_date);
			if (access (file_path, R_OK) == 0) unlink(file_path);

			//get src file to compress
			if(upload_pat[m_code].cnt > 0){
				if(compress_db_files(&upload_pat[m_code].items, upload_pat[m_code].cnt, file_path) == RET_FAIL){
					__AGENT_DBGP(__FAIL, "ERR : Upload File Compress Fail!!\n");
					return RET_FAIL;
				}
			}else{
				__AGENT_DBGP(__FAIL, "ERR : Can't find src file to Compress !!\n");
				return RET_FAIL;
			}
			snprintf(data->file_list.src_file, 512, "%s", file_path);
			return RET_OK;
			break;
		default:
			data->status_code = STAT_NOT_SUPPORT_CODE_ERR;
			return RET_FAIL;
			break;
	}
}

/************************************************/
/* Pattern Down																	*/
/************************************************/

int setup_pattern (DATA_t *data)
{
	PROC_t	p;

	int category = STATUS_3R_NONE;
	int category_flag = 0;

	int result = 0;
	int	rcode = STATUS_FAIL;

	memset (&p, '\0', sizeof(p));
	p.data = data;

	if (strlen((const char *)data->file_list.src_file) == 0) {
		result = STAT_NO_FILE_INFORMATION;
		goto setup_pattern_error;
	}

	switch (data->type_code[0]) {
		case '1':	//download
			if (data->type_code[2] == '0') category = STATUS_3R_PATTERN;
			else if (data->type_code[2] == '1') category = STATUS_3R_RL;
			p.process_func = process_PATTERN_UPDATE;
			break;
		case '2':	//upload
		default:
			//error
			p.process_func = NULL;
			result = STAT_NOT_SUPPORT_CODE_ERR;
			break;
	}
	if (result) goto setup_pattern_error;

	audit_func_insert (data, category, category_flag);

	if (p.process_func && category != STATUS_3R_NONE) {
		//TODO Process
		result = p.process_func (&p);
		if (result != 0) {
			rcode = STATUS_FAIL;
			unlink (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP);
		} else {
			rcode = STATUS_SUCCESS;
		}
	} else {
		result = STAT_PROC_CALL_ERR;
	}

setup_pattern_error:
	data->status_code = result;

	return rcode;
}

int process_PATTERN_UPDATE (PROC_t *proc)
{
	int ret = STAT_NOT_SUPPORT_CODE_ERR;
	DATA_t *data = proc->data;

	switch (data->type_code[4]) {
		case '0':	/* LUS */
		case '9':	/* MUS */
			ret = Local_Pattern_Update_from_DIR (proc);
			break;
		default:
			break;
	}

	return ret;
}

/************************************************/
/* sn_rmsa Patch																*/
/************************************************/

int rmsa_patch_func (DATA_t *data)
{
	PROC_t	p;
	PROC_t *proc = &p;
	FILE_t *flist = &(data->file_list);

	DBB_t  *dbb;

	struct dirent *item;
	DIR *dp;
	struct stat item_stat;

	int result = 0;
	int	rc, rcode = STATUS_FAIL;

	memset (&p, '\0', sizeof(p));
	p.data = data;

	if (strlen((const char *)data->file_list.src_file) == 0) {
		result = STAT_NO_FILE_INFORMATION;
		goto rmsa_patch_error;
	}

	audit_func_insert (data, STATUS_3R_SN_SPU, 0);

	//TODO Make Unzip dir
	snprintf(proc->update_path, sizeof(proc->update_path)
			, "%s/%s", LIVEUPDATE_PATH, data->release_date);

	snprintf (proc->unzip_path, sizeof (proc->unzip_path)
			, "%s/tmp", proc->update_path);

	if(access (proc->unzip_path, F_OK) == 0) { 
		file_remove (proc->unzip_path);
	}
	mkdir (proc->unzip_path, 0755);

	//TODO Unzip
	if (file_unzip ((char *)flist->src_file, proc->unzip_path) < 0) {
		__AGENT_DBGP(__FAIL, "Worker: DB File Unzip Fail\n");
		result = STAT_FILE_UNZIP_ERR;
		goto rmsa_patch_error;
	}

	//TODO Alloc list
	proc->dbb = (DBB_t *) malloc (sizeof(DBB_t) * MAX_CATEGORY_CNT);	
	if (proc->dbb == NULL) {
		result = STAT_MEMORY_ALLOC_ERR;
		goto rmsa_patch_error;
	}
	memset (proc->dbb, '\0', (sizeof(DBB_t) * MAX_CATEGORY_CNT));

	//TODO Read Unzip Dir
	dp = opendir(proc->unzip_path);
	if (dp == NULL) {
		__AGENT_DBGP (__FAIL, "Worker: Directory Open Fail(%s)\n", proc->unzip_path);
		result = STAT_FILE_PATH_ERR;
		goto rmsa_patch_error;
	}

	dbb = proc->dbb;
	while ((item = readdir(dp)) != NULL) {
		if (strcmp(item->d_name, "..") == 0 ||
				strcmp(item->d_name, ".") == 0 ) continue;

		memset (dbb, '\0', sizeof (DBB_t));
		snprintf (dbb->src_file, sizeof (dbb->src_file)
				, "%s/%s"
				, proc->unzip_path, item->d_name);

		memset(&item_stat, '\0', sizeof(struct stat));
		stat(dbb->src_file, &item_stat);
		if (S_ISDIR(item_stat.st_mode)) continue;

		dbb->update = 1;

		//TODO Set Name
		if (strncmp (item->d_name
					, FILE_NAME_RMSA_DAT
					, strlen (FILE_NAME_RMSA_DAT)) == 0) { 

			snprintf (dbb->dst_file, sizeof (dbb->dst_file)
					, "../config/%s"
					, item->d_name);
		} else if (strncmp (item->d_name
					, FILE_NAME_RMSA
					, strlen (FILE_NAME_RMSA)) == 0) { 

			snprintf (dbb->dst_file, sizeof (dbb->dst_file)
					, "../sniper/%s"
					, item->d_name);
		} else if (strncmp (item->d_name
					, FILE_NAME_SN_SPU
					, strlen (FILE_NAME_SN_SPU)) == 0) { 

			snprintf (dbb->dst_file, sizeof (dbb->dst_file)
					, "../sniper/%s"
					, item->d_name);
		} else {
			result = STAT_FILE_NAME_ERR;
			break;
		}

		//TODO Backup
		snprintf (dbb->bak_file, sizeof (dbb->bak_file)
				, "%s.bak"
				, dbb->dst_file);
		if (access (dbb->dst_file, F_OK) == 0) {
			if (file_copy (dbb->dst_file, dbb->bak_file) < 0) {
				__AGENT_DBGP(__FAIL,"Worker: File Copy Fail (%s)\n", dbb->dst_file);
				result = STAT_FILE_COPY_ERR;
				break;
			}
		}

		proc->update_count++;

		//TODO Copy
		if (access (dbb->dst_file, F_OK) == 0) unlink (dbb->dst_file);
		if (file_copy ((const char *)dbb->src_file
					, (const char *)dbb->dst_file) < 0) {
			__AGENT_DBGP(__FAIL,"Worker: File Copy Fail (%s)\n", dbb->src_file);
			result = STAT_FILE_COPY_ERR;
			break;
		}

		//TODO Chmod
		if (chmod (dbb->dst_file, 0700)) {
			__AGENT_DBGP(__FAIL,"Worker: File Mode Change Fail (%s)\n", dbb->dst_file);
			result = STAT_FILE_COPY_ERR;
			break;
		}

		dbb++;
	}	//end while

	if (proc->update_count == 0) result = STAT_NO_FILE_INFORMATION;

	if (result) {
		rcode = STATUS_FAIL;
		if ((rc = rollback_all (proc)) == 0)
			__AGENT_DBGP(__FAIL, "Worker: Rollback Fail\n");
	} else {
		rcode = STATUS_SUCCESS;
	}

	backup_clear (proc);

rmsa_patch_error:
	data->status_code = result;

	if (dp) closedir(dp);
	if (proc->dbb) free (proc->dbb);

	return rcode;
}

/************************************************/
/* Rec Engine																		*/
/************************************************/

int setup_rec_engine (DATA_t *data)
{
	PROC_t	p;

	int category = STATUS_3R_NONE;
	int category_flag = 0;

	int result = 0;
	int	rcode = STATUS_FAIL;

	memset (&p, '\0', sizeof(p));
	p.data = data;

	if (strlen((const char *)data->file_list.src_file) == 0) {
		result = STAT_NO_FILE_INFORMATION;
		goto rec_engine_error;
	}

	switch (data->type_code[2]) {
		case '1':	/* MR */
			category = STATUS_3R_MR;
			category_flag = 13;
			p.process_func = process_MR_CLAMAV;
			break;
		case '2':	/* UR */
			category_flag = 12;
			category = STATUS_3R_UR;
			break;
		case '3':	/* AR */
			category_flag = 11;
			category = STATUS_3R_AR;
			p.process_func = process_PATTERN_UPDATE;
			break;
		case '4':	/* IPR */
			category_flag = 10;
			category = STATUS_3R_IPR;
			p.process_func = process_PATTERN_UPDATE;
			break;
		default:
			//error
			p.process_func = NULL;
			result = STAT_NOT_SUPPORT_CODE_ERR;
			break;
	}
	if (result) goto rec_engine_error;

	audit_func_insert (data, category, category_flag);

	if (p.process_func && category != STATUS_3R_NONE) {
		//TODO Process
		result = p.process_func (&p);
		if (result != 0) {
			rcode = STATUS_FAIL;
			unlink (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP);
		} else {
			rcode = STATUS_SUCCESS;
		}
	} else {
		result = STAT_PROC_CALL_ERR;
	}

rec_engine_error:
	data->status_code = result;

#if 0
	//TODO audit
	audit_func_update (data, category);
#endif

	return rcode;
}

int compile_func (char *category, char *release_date)
{
	if (insert_loadpattern_file (category, release_date) < 0) {
		__AGENT_DBGP(__FAIL,"Worker: Insert Loadlock File Fail\n");
		return 1;
	}

	return 0;
}

int compile_func_clear ()
{
	if (access (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP, F_OK) == 0) 
		file_move (LIVEUPDATE_PATTERN_LOAD_FNAME
				, LIVEUPDATE_PATTERN_LOAD_FNAME_TMP);

	return 0;
}

int process_IPR (PROC_t *proc)
{
	int ret = STAT_NOT_SUPPORT_CODE_ERR;
	DATA_t *data = proc->data;

	__AGENT_DBGP(__INFO, "IPR Update Start\n");

	switch (data->type_code[4]) {
		case '0':	/* PATTERN */
		case '9':	/* PATTERN(manual) */
			ret = process_IPR_PATTERN (proc);
			break;
		case '1':	/* POLICY */
			ret = process_IPR_POLICY (proc);
			break;
		default:
			//error
			break;
	}

	__AGENT_DBGP(__INFO, "IPR Update Done\n");

	return ret;
}

int process_IPR_PATTERN (PROC_t *proc)
{
	int ret = 0;
	char filename [100] = {0,};
	char tmpdir [1024]= {0,};
	char tmpcmd [1024]= {0,};

	DATA_t *data = proc->data;
	FILE_t *flist;

	char *p;

	flist = &(data->file_list);

	p = strrchr ((char *)flist->src_file, '/');
	if (p == NULL) {
		ret = 1;
		goto ipr_error;
	}

	memcpy (filename, p, strlen(p));

	//TODO File Name Check
	snprintf ((char *)flist->dst_file, sizeof(flist->dst_file)
			, "%s/%s"
			, COMPILE_PATH_IPR, filename);

	//TODO Backup
	snprintf (tmpdir, sizeof (tmpdir), "%s/tmp/", COMPILE_PATH_IPR);
	if (access (tmpdir, F_OK)) mkdir (tmpdir, 0755);
	snprintf (tmpcmd, sizeof (tmpcmd)
			, "/bin/mv -f ./%s/*_%s %s"
			, COMPILE_PATH_IPR, FILE_NAME_IPR_PATTERN, tmpdir);
	system (tmpcmd);

	snprintf ((char *)flist->tmp_file, sizeof (flist->tmp_file)
			, "%s.bak", flist->dst_file);

	if (access((const char *)flist->dst_file, F_OK) == 0)
		unlink (flist->dst_file);

	//TODO move
	if (file_copy ((const char *)flist->src_file
				, (const char *)flist->dst_file) < 0) {
		__AGENT_DBGP(__FAIL,"Worker: File Copy Fail\n");
		return STAT_FILE_COPY_ERR;
	}

	//TODO unzip
	if (file_unzip ((char *)flist->dst_file, COMPILE_PATH_IPR) < 0) {
		snprintf (tmpcmd, sizeof (tmpcmd)
				, "/bin/mv -f ./%s/*_%s ./"
				, tmpdir, FILE_NAME_IPR_PATTERN);
		system (tmpcmd);
		__AGENT_DBGP(__FAIL, "Worker: File Unzip Fail\n");
		return STAT_FILE_UNZIP_ERR;
	}

	__AGENT_DBGP(__OK, "Worker: File Copy Complete(%s)\n", flist->dst_file);

ipr_error:
	//TODO Load Lock
	if (compile_func (CATEGORY_STR_IPR, data->release_date)) {
		ret = STAT_COMPILE_ERR;
	}

	snprintf (tmpcmd, sizeof (tmpcmd), "/bin/rm -rf %s", tmpdir);
	system (tmpcmd);
	cleanup_func (&data->file_list);

	return ret;
}

int process_IPR_POLICY (PROC_t *proc)
{
	int ret = 0, rc;

	DATA_t *data = proc->data;
	FILE_t *flist;

	flist = &(data->file_list);

	//TODO File Name Check
	snprintf ((char *)flist->dst_file, sizeof(flist->dst_file)
			, "%s/%s"
			, COMPILE_PATH_IPR, LUS_DB_IPR_POLICY);

	//TODO Backup
	snprintf ((char *)flist->tmp_file, sizeof (flist->tmp_file)
			, "%s.bak", flist->dst_file);

	if (access((const char *)flist->dst_file, F_OK) == 0) {
		rc = file_move ((const char *)flist->tmp_file, (const char *)flist->dst_file);
		__AGENT_DBGP (__INFO, "File Move (%d)\n", rc);
	}

	//TODO unzip
	if (file_unzip ((char *)flist->src_file, COMPILE_PATH_IPR) < 0) {
		file_recover ((const char *)flist->tmp_file, (const char *)flist->dst_file);
		__AGENT_DBGP(__FAIL,"Worker: File Unzip Fail\n");
		return STAT_FILE_UNZIP_ERR;
	}

	__AGENT_DBGP(__OK,"Worker: File Copy Complete(%s)\n", flist->dst_file);

	//TODO Load Lock
	if (compile_func (CATEGORY_STR_IPR, data->release_date)) {
		ret = STAT_COMPILE_ERR;
	}
	cleanup_func (&data->file_list);

	return ret;
}

int process_AR (PROC_t *proc)
{
	int ret = STAT_NOT_SUPPORT_CODE_ERR;
	DATA_t *data = proc->data;

	__AGENT_DBGP(__INFO, "AR Update Start\n");

	switch (data->type_code[4]) {
		case '0':	/* PATTERN */
		case '9':	/* PATTERN(manual) */
			ret = process_AR_PATTERN (proc);
			break;
		case '1':	/* POLICY */
			ret = process_AR_POLICY (proc);
			break;
		default:
			//error
			break;
	}

	__AGENT_DBGP(__INFO, "AR Update Done\n");

	return ret;
}

int process_AR_PATTERN (PROC_t *proc)
{
	int rc, ret = 0;

	DATA_t *data = proc->data;
	FILE_t *flist;

	flist = &(data->file_list);

	//TODO File Name Check
	snprintf ((char *)flist->dst_file, sizeof(flist->dst_file)
			, "%s/%s"
			, COMPILE_PATH_CONFIG, FILE_NAME_AR_PATTERN);

	//TODO Backup
	snprintf ((char *)flist->tmp_file, sizeof (flist->tmp_file)
			, "%s.bak", flist->dst_file);

	if (access((const char *)flist->dst_file, F_OK) == 0) {
		rc = file_move ((const char *)flist->tmp_file, (const char *)flist->dst_file);
		__AGENT_DBGP (__INFO, "File Move (%d)\n", rc);
	}

	//TODO unzip
	if (file_unzip ((char *)flist->src_file, COMPILE_PATH_CONFIG) < 0) {
		file_recover ((const char *)flist->tmp_file, (const char *)flist->dst_file);
		__AGENT_DBGP(__FAIL,"Worker: File Unzip Fail\n");
		return STAT_FILE_UNZIP_ERR;
	}

	__AGENT_DBGP(__OK,"Worker: File Copy Complete(%s)\n", flist->dst_file);

	//TODO Load Lock
	if (compile_func (CATEGORY_STR_AR, data->release_date)) {
		ret = STAT_COMPILE_ERR;
	}
	cleanup_func (&data->file_list);

	return ret;
}

int process_AR_POLICY (PROC_t *proc)
{
	int i, ret = 0;
	int count = 0;
	char filename [100];

	DATA_t *data = proc->data;
	FILE_t *tar, *flist, *fl;		//free (rlist);

	char *p;

	tar = &(data->file_list);

	//TODO Unzip src file
	if (access (LIVEUPDATE_PATH_TEMP, F_OK)) {
		mkdir (LIVEUPDATE_PATH_TEMP, 0755);
	} else {
		file_remove (LIVEUPDATE_PATH_TEMP);
	}

	if (file_unzip ((char *)tar->src_file, LIVEUPDATE_PATH_TEMP) < 0) {
		__AGENT_DBGP(__FAIL,"Worker: File Unzip Fail\n");
		return STAT_FILE_UNZIP_ERR;
	}

	fl = get_file_list (LIVEUPDATE_PATH_TEMP);
	if (fl == NULL) return STAT_FILE_ARCHIVE_ERR;

	flist = fl;
	for (i = 0; i < MAX_FILELIST_COUNT; i++) {

		if (strlen((char *)flist->src_file) == 0) break;

		//TODO split here
		p = strrchr ((char *)flist->src_file, '/');
		if (p == NULL) { ret = STAT_FILE_ACCESS_ERR; break; }
		else p++;

		memset (filename, '\0', sizeof (filename));
		memcpy (filename, p, strlen(p));

		if(strncmp(filename
					, LUS_DB_AR_POLICY
					, strlen(LUS_DB_AR_POLICY)) == 0) {

			file_setup (flist, COMPILE_PATH_CONFIG, LUS_DB_AR_POLICY, 1); 

			if (file_move (flist->dst_file, flist->src_file) < 0) {
				__AGENT_DBGP(__FAIL,"Worker: File Copy Fail\n");
				ret = STAT_FILE_COPY_ERR; break;
			}

			__AGENT_DBGP(__OK, "Worker: AR Policy Update SUccess\n");

		} else if (strncmp(filename
					, LUS_DB_RULESET_EIP3
					, strlen(LUS_DB_RULESET_EIP3)) == 0) {

			file_setup (flist, COMPILE_PATH_CONFIG, LUS_DB_RULESET_EIP3, 0); 

			if (update_eip_list (flist) < 0) {
				__AGENT_DBGP(__FAIL,"Worker: Eip Update Fail\n");
				ret = STAT_EIP_UPDATE_ERR; break;
			}

			__AGENT_DBGP(__OK, "Worker: EIP Update SUccess\n");

		} else {
			__AGENT_DBGP(__FAIL,"Worker: Unknown File (%s)\n", filename); 
			ret = STAT_FILE_ACCESS_ERR; break;
		}

		cleanup_func (flist); flist++;
	}
	count = i;

	if (count && ret == 0) {
		//TODO Load Lock
		if (compile_func (CATEGORY_STR_AR, data->release_date)) {
			ret = STAT_COMPILE_ERR;
		}
	} else if (ret > 1) {
		//TODO Recover
		flist = fl;
		for (i = 0; i < count; i++) {
			file_recover (flist->tmp_file, flist->dst_file);
			cleanup_func (flist); flist++;
		}
		__AGENT_DBGP(__OK, "Worker: Rollback Success(%d)\n", count);
	}

	//TODO Remove DIR
	if (access (LIVEUPDATE_PATH_TEMP, F_OK) == 0) 
		file_remove (LIVEUPDATE_PATH_TEMP);

	if (fl) free (fl);

	return ret;
}

int process_MR_CLAMAV (PROC_t *proc)
{
	char filename [100] = {0,};

	DATA_t *data = proc->data;
	FILE_t *flist;

	char *p, *q;

	int  ret = 0;

	__AGENT_DBGP(__INFO, "MR Update Start\n");

	flist = &(data->file_list);

	//TODO Parse File Name
	p = strrchr ((char *)flist->src_file, '/');	p++;
	if (p == NULL) return STAT_FILE_ACCESS_ERR;
	memcpy (filename, p, strlen(p));

	if (access (COMPILE_PATH_CLAMAV, F_OK)) mkdir (COMPILE_PATH_CLAMAV, 0755);

	//TODO unzip
	if (file_unzip ((char *)flist->src_file, COMPILE_PATH_CLAMAV) < 0) {
		__AGENT_DBGP (__FAIL, "Worker: File Unzip Fail");
		return STAT_FILE_UNZIP_ERR;
	}
	__AGENT_DBGP(__OK,"Worker: File Copy Complete(%s)\n", flist->dst_file);

	//TODO Remove Agent Download
	if (create_mrlock_file (data->update_date)) {
		__AGENT_DBGP(__FAIL,"Worker: Make MR Lock File Fail\n");
	}

	if (compile_func (CATEGORY_STR_MR, data->release_date)) 
		ret = STAT_COMPILE_ERR;
	cleanup_func (&data->file_list);

	__AGENT_DBGP(__INFO, "MR Update Done\n");

	return ret;
}

int update_eip_list (FILE_t *flist)
{
	char sql[8192];
	char db[1024] = {0,};

	snprintf (db, sizeof(db)
			, "%s/%s"
			, COMPILE_PATH_CONFIG, LUS_DB_RULESET_EIP3);

	memset (sql, '\0', sizeof(sql));

	//TODO make query		
	snprintf (sql, sizeof(sql),
			" ATTACH '%s' as update1;"
			" INSERT OR REPLACE INTO master( "
			" category"
			",code"
			",flag"
			",proto"
			",s_port"
			",d_port"
			",s_network"
			",s_netmask"
			",s_broadcast"
			",d_network"
			",d_netmask"
			",d_broadcast"
			",v_id"
			") "
			"SELECT"
			" category"
			",code"
			",flag"
			",proto"
			",s_port"
			",d_port"
			",s_network"
			",s_netmask"
			",s_broadcast"
			",d_network"
			",d_netmask"
			",d_broadcast"
			",v_id"
			" FROM update1.master as u"
			" WHERE u.category='%d';"
			, flist->src_file
			, ATTACK_AR_CTL
	);

	if (ExecDBQuery (db, sql) < 0) return -1;

	return 0;
}

/************************************************/
/* Load Pattern File														*/
/************************************************/

int create_mrlock_file (char *update_date)
{
	struct tm *tp;

	FILE 			*fp;

	fp = fopen (LIVEUPDATE_MRLOCK_FILE, "w");
	if (fp == NULL) return -1;

	fwrite (update_date, 1, strlen(update_date), fp);
	fwrite ("\n", 1, 1, fp);
	fclose (fp);

	return 0;
}

/**
 * @brief 패턴업데이트의진행을 알리는 loadlock.tmp 을 만든다
 *
 * @param update_date 
 *
 * @return 
 */
int create_loadpattern_file (char *update_date)
{
	FILE 			*fp;

	if (access (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP, F_OK) == 0) return 1;

	fp = fopen (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP, "w");
	if (fp == NULL) return -1;

	fwrite (update_date, 1, strlen(update_date), fp);
	fwrite ("\n", 1, 1, fp);
	fclose (fp);

	return 0;
}

int insert_loadpattern_file (char *category, char *release_date)
{
	FILE 			*fp;

	if ((fp = fopen (LIVEUPDATE_PATTERN_LOAD_FNAME_TMP, "a+")) == NULL)
		return -1;

	fwrite (category, 1, strlen(category), fp);
	fwrite ("|", 1, 1, fp);
	fwrite (release_date, 1, strlen(release_date), fp);
	fwrite ("\n", 1, 1, fp);
	fclose (fp);

	return 0;
}

/************************************************/
/* Audit 																				*/
/************************************************/

int createTable_audit (char *db)
{
	char sql[8192];

	memset (sql, '\0', sizeof(sql));

	snprintf (sql, sizeof (sql),
			"CREATE TABLE master( \n"
			" log_date                TEXT\n"
			",platform_version        TEXT\n"
			",category                 INT\n"
			",category_flag            INT\n"
			",update_date             TEXT\n"
			",release_date            TEXT\n"
			",filename                TEXT\n"
			",filesize                 INT\n"
			",type_code                INT\n"
			",status_code              INT\n"
			",status_info   	        TEXT\n"
			",compile_date            TEXT\n"
			",compile_result           INT\n"
			",update_type							TEXT\n"
			");\n"
			"CREATE UNIQUE INDEX _master_key ON master (update_date, release_date, category);\n"
			);

	if (ExecDBQuery (db, sql) < 0) return -1;

	return 0;
}

int set_ids_category (int category)
{
	int ret = 0;

	switch (category) {
		case STATUS_3R_PATTERN:		ret = ATTACK_DOS; break;
		case STATUS_3R_IPR: 			ret = ATTACK_REPUTATION_BASE; break;
		case STATUS_3R_MR:				ret = ATTACK_MR_CTL; break;
		case STATUS_3R_UR:				ret = ATTACK_UR_CTL; break;
		case STATUS_3R_AR:				ret = ATTACK_AR_CTL; break;
		case STATUS_3R_RL:				ret = ATTACK_NQOS; break;
		default: break;
	}

	return ret;
}

int make_log_path(struct tm	*tp, char *path, int path_size)
{
	char db[1024] = {0,};
	char dir[1024] = {0,};

	snprintf (dir, sizeof(dir), "%s/%s", COMPILE_PATH_DBMS, "rms");
	if (access (dir, F_OK)) mkdir (dir, 0755);

	// ./dbms/log/YYYY
	sprintf(dir, "%s/%04d", dir, tp->tm_year);
	if (access(dir, R_OK)) mkdir (dir, 0700); 

	// ./dbms/log/YYYY/MM
	sprintf(dir, "%s/%02d", dir, (tp->tm_mon + 1));
	if (access(dir, R_OK)) mkdir (dir, 0700); 

	// ./dbms/log/YYYY/MM/DD
	sprintf(dir, "%s/%02d", dir, tp->tm_mday);
	if (access(dir, R_OK)) mkdir (dir, 0700); 

	snprintf (db, sizeof(db), "%s/%s_%4d%02d%02d.dbb"
			, dir, FILE_NAME_AUDIT
			, tp->tm_year, tp->tm_mon + 1, tp->tm_mday);

	memset(path, '\0', path_size);
	strncpy(path, db, path_size);

	return 0;
}

int audit_func_update (DATA_t *data, task_arg_t *up_args)
{
	char sql[8192];

	if (access (data->audit_db, F_OK) != 0) return -1;

	memset (sql, '\0', sizeof(sql));
	snprintf (sql, sizeof(sql),
			"UPDATE master SET"
			" status_code=%d"
			" status_info='%s'"
			" WHERE category=%d AND update_date='%s' AND release_date='%s';"
			, data->status_code	
			,	set_ids_category (up_args->category)
			, up_args->status_info	
			, data->update_date
			, data->release_date
			);

	if (ExecDBQuery (data->audit_db, sql) < 0) return -1;

	return 0;
}

int audit_func_insert (DATA_t *data, int category, int category_flag)
{
	char sql[8192];

	struct tm *tp;
	time_t    timeval;
	char    	rdate[64];

	timeval = time (NULL);
	tp = localtime (&timeval);
	if (tp->tm_year > 90)
		tp->tm_year += 1900;
	else tp->tm_year += 2000;

	//date over - do not considered
	make_log_path (tp, data->audit_db, sizeof(data->audit_db));
	if (access (data->audit_db, F_OK) != 0) {
		if (createTable_audit (data->audit_db)) return -1;
	}	

	snprintf (rdate, sizeof (rdate)
			, "%04d/%02d/%02d %02d:%02d:%02d"
			, tp->tm_year, tp->tm_mon + 1, tp->tm_mday
			, tp->tm_hour, tp->tm_min, tp->tm_sec);

	memset (sql, '\0', sizeof(sql));
	snprintf (sql, sizeof(sql),
			" INSERT OR REPLACE INTO master( "
			" log_date"
			",platform_version"
			",category"
			",category_flag"
			",update_date"
			",release_date"
			",filename"
			",filesize"
			",type_code"
			",status_code"
			",status_info"
			",compile_date"
			",compile_result"
			",update_type"
			") VALUES ( "
			"'%s', '%s', '%d', '%d', '%s', '%s', '%s', %lu, '%d'"
			", -1, ' ', ' ', -1, 'RMS'"
			");"
			, rdate
			, data->platform_version
			,	set_ids_category (category)
			, category_flag
			, g_update_date
			, data->release_date
			, data->file_list.src_file
			, data->file_list.file_size
			, atoi ((const char *)data->type_code)
			);

	if (ExecDBQuery (data->audit_db, sql) < 0) return -1;

	return 0;
}

/************************************************/
/* Util 																				*/
/************************************************/

int file_copy (const char *src, const char *dst)
{
	FILE *in, *out;
	char *buf;
	long filesize;
	size_t	len;

	if (!strcmp (src, dst))	return -1;

	if ((in = fopen (src, "rb")) == NULL) return -1;
	if ((out = fopen (dst, "wb")) == NULL) {
		fclose (in);
		return -1;
	}

	fseek (in, 0, SEEK_END);
	filesize = ftell (in);	
	fseek (in, 0, SEEK_SET);

	buf = (char *) malloc (filesize);
	if(buf == NULL) {
		fclose (in);
		fclose (out);
		return -1;
	}
	memset (buf, '\0', filesize);

	len = fread (buf, sizeof (char), filesize, in);
	if (fwrite (buf, sizeof (char), len, out) == 0) {
		fclose (in);
		fclose (out);
		free (buf);
		return -1;
	}	

	fclose (in);
	fclose (out);
	free (buf);

	return 0;
}

int file_recover (const char *tmp, const char *org)
{
	char tmp_cmd 	[1024];

	if (access (tmp, F_OK) != 0) {
		unlink (org);		// ?
		return -1;
	}

	unlink (org);
	file_move (org, tmp);

	if (access (tmp, F_OK) == 0) unlink (tmp);

	return 0;
}

static int copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	int64_t offset;

	for (;;) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)  return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK)  return (r);
	}
}

int compress_db_files(char** src_file, int cnt, char* dst_path)
{
	char buff[8192];
	int r;
	mode_t m;
	struct archive *ina;
	struct archive *outa;
	struct archive_entry *entry;
	int i=0; 
	int	fd;
	ssize_t len;

	outa = archive_write_new();
	archive_write_set_compression_gzip(outa);
	archive_write_set_format_ustar(outa); 
	archive_write_open_filename(outa, dst_path);

	while (i < cnt) {
		ina = archive_read_disk_new();
		int r;

		r = archive_read_disk_open(ina, *(src_file+i));
		if (r != ARCHIVE_OK) {
			__AGENT_DBGP(__ERR, "%s\n", archive_error_string(ina));
			return RET_FAIL;
		}

		for (;;) {
			entry = archive_entry_new();
			r = archive_read_next_header2(ina, entry);
			if (r == ARCHIVE_EOF)
				break;
			if (r != ARCHIVE_OK) {
				__AGENT_DBGP(__ERR, "%s\n", archive_error_string(ina));
				break;
			}
			archive_read_disk_descend(ina);
			__AGENT_DBGP(__INFO, "%s\n", archive_entry_pathname(entry));

			r = archive_write_header(outa, entry);
			if (r < ARCHIVE_OK) {
				__AGENT_DBGP(__FAIL,"ERR :  Write to Compress dst file!\n");
			}
			if (r == ARCHIVE_FATAL){
				__AGENT_DBGP(__FAIL,"ERR : Compress update file Fail!!\n");
				return RET_FAIL;
			}
			if (r > ARCHIVE_FAILED) {
				fd = open(archive_entry_sourcepath(entry), O_RDONLY);
				len = read(fd, buff, sizeof(buff));
				while (len > 0) {
					archive_write_data(outa, buff, len);
					len = read(fd, buff, sizeof(buff));
				}
				close(fd);
			}
			archive_entry_free(entry);
		}
		archive_read_close(ina);
		archive_read_free(ina);
		i++;
	}
	archive_write_close(outa);
	archive_write_free(outa);

	return RET_OK;
}

int file_unzip (char *archive, char *dst)
{
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int r;

	int flags;

	char tmp_file [1024];
	char dst_file [1024] = {0,};

	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	flags |= ARCHIVE_EXTRACT_ACL;
	flags |= ARCHIVE_EXTRACT_FFLAGS;

	a = archive_read_new();
	archive_read_support_filter_gzip(a);
	archive_read_support_format_tar(a);

	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);

	if (archive != NULL && strcmp(archive, "-") == 0)
		archive = NULL;

	if ((r = archive_read_open_filename(a, archive, 10240))) {
		__AGENT_DBGP(__ERR, "%s\n", archive_error_string(a));
		return -1;
	}

	if (access (dst, F_OK) != 0) mkdir (dst, 0755);

	for (;;) {
		memset (tmp_file, '\0', sizeof(tmp_file));	
		memset (dst_file, '\0', sizeof(dst_file));	

		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF) break;
		if (r != ARCHIVE_OK) {
			__AGENT_DBGP(__ERR, "%s\n", archive_error_string(a));
			return -1;
		}

		snprintf (tmp_file, sizeof(tmp_file)
				, "%s/%s.tmp", dst, archive_entry_pathname(entry));
		snprintf (dst_file, sizeof(dst_file)
				, "%s/%s", dst, archive_entry_pathname(entry));
		archive_entry_set_pathname(entry, tmp_file);

		r = archive_write_header(ext, entry);
		if (r != ARCHIVE_OK) {
			__AGENT_DBGP(__ERR, "%s\n", archive_error_string(a));
			return -1;
		}
		else {
			r = copy_data(a, ext);
			if (r != ARCHIVE_OK) {
				__AGENT_DBGP(__ERR, "%s\n", archive_error_string(a));
				return -1;
			}
		}

		file_move (dst_file, tmp_file);
		__AGENT_DBGP(__INFO, "Unzip (%s)\n", tmp_file);
	}

	archive_read_close(a);
	archive_read_free(a);

	archive_write_close(ext);
	archive_write_free(ext);

	return 0;
}

int file_move (const char *dst, const char *src)
{
	int rc = 0;

	if (access (src, F_OK) != 0) return -1;
	if (access (dst, F_OK) == 0) unlink (dst);	

	if (rename (src, dst) != 0 ) {
		return -1;
	} 

	return rc;
}  

int file_remove (char *path)
{
	struct dirent *item;
	DIR *dp;
	char next[1024] = {0};

	if (strlen (path) < 4) return -1;

	dp = opendir(path);
	if (dp == NULL) return -1;

	while ((item = readdir(dp)) != NULL) {
		if (strcmp(item->d_name, "..") == 0 ||
				strcmp(item->d_name, ".") == 0 ) continue;
		snprintf(next, sizeof(next), "%s/%s", path, item->d_name);

		struct stat item_stat;
		stat(next, &item_stat);

		if (S_ISDIR(item_stat.st_mode)) {
			__AGENT_DBGP(__INFO, "cd %s\n", next);
			if (file_remove (next) == -1) {
				return -1;
			}
		} else {
			memset(next, 0x00, sizeof(next));
			snprintf(next, sizeof(next), "%s/%s", path, item->d_name);
			__AGENT_DBGP(__INFO, "rm %s\n", next);
			if (unlink(next) == -1) return -1;
		}
	}
	__AGENT_DBGP(__INFO, "rmdir %s\n", path);
	rmdir(path);

	closedir(dp);

	return 0;
}

int file_setup (FILE_t *flist, char *dir_path, char *filename, int move)
{
	if (flist == NULL) return -1;

	snprintf ((char *)flist->dst_file, sizeof(flist->dst_file)
			, "%s/%s", dir_path, filename);

	snprintf ((char *)flist->tmp_file, sizeof (flist->tmp_file)
			, "%s.bak", flist->dst_file);

	if (access((const char *)flist->dst_file, F_OK) == 0) {
		if (move) {
			file_move ((const char *)flist->tmp_file
					, (const char *)flist->dst_file);
		} else {
			file_copy ((const char *)flist->dst_file
					, (const char *)flist->tmp_file);
		}
	}

	return 0;
}

void file_chase (char *filename)
{
	if (access (filename, F_OK)) {
		__AGENT_DBGP(__INFO, "File do not exists (%s)\n", filename);
	} else {
		__AGENT_DBGP(__INFO, "File exists(%s)\n", filename);
	}
}

#if 0
void data_print (DATA_t *data)
{
	FILE_t *flist = &(data->file_list);

	fprintf (stderr, "=========================================\n");
	fprintf (stderr, "ip addr        : %s\n"	, data->dev.ip_addr);
	fprintf (stderr, "port           : %s\n"	, data->dev.port);
	fprintf (stderr, "type_code      : %s\n"	, data->type_code);
	fprintf (stderr, "src_file       : %s\n"	, flist->src_file);
	fprintf (stderr, "dst_file       : %s\n"	, flist->dst_file);
	fprintf (stderr, "file_size      : %lu\n"	, flist->file_size);
	fprintf (stderr, "help_file      : %s\n"	, flist->help_file);
	fprintf (stderr, "help_size      : %lu\n"	, flist->help_size);
	fprintf (stderr, "update_date    : %s\n"	, data->update_date);
	fprintf (stderr, "lang_code      : %d\n"	, data->lang_code);
	fprintf (stderr, "audit_db       : %s\n"	, data->audit_db);
	fprintf (stderr, "=========================================\n");
}
#endif

int cleanup_func (FILE_t *flist)
{
	if (access((const char *)flist->tmp_file, F_OK) == 0) 
		unlink ((const char *)flist->tmp_file);

	return 0;
}

static char get_single_char (int n)
{
	char a = '0';

	switch (n) {
		case 0:	a = '0'; break;
		case 1:	a = '1'; break;
		case 2:	a = '2'; break;
		case 3:	a = '3'; break;
		case 4:	a = '4'; break;
		case 5:	a = '5'; break;
		case 6:	a = '6'; break;
		case 7:	a = '7'; break;
		case 8:	a = '8'; break;
		case 9:	a = '9'; break;
		default: break;
	}

	return a;
}

static void code_to_str (int code, char *string, char size)
{
	int i, n, m = 10000;

	if (code > 99999) return;

	for (i = 0; i < size - 1; i++) {
		n = code / m; string[i] = get_single_char (n);
		code -= (n * m);
		m /= 10;
	}	

	string[size - 1] = '\0';

	return;
}

static int str_to_code (char *string)
{
	int code = 0;

	code = atoi (string);	
	if (code > 99999) code = 0;

	return code;
}

FILE_t *get_file_list (char *dir)
{
	int count = 0;

	DIR *dp = NULL;
	struct dirent *dirp = NULL;

	FILE_t *file_list, *fl = NULL;

	if((dp = opendir (dir)) == NULL) {
		__AGENT_DBGP(__ERR, "Worker: Directory Open Fail(%s)", dir);
		return NULL;
	}

	file_list = (FILE_t *) malloc (MAX_FILELIST_COUNT * sizeof (FILE_t));
	if (file_list == NULL) return NULL;

	fl = file_list;
	while((dirp = readdir(dp)) != NULL) {
		if(!strncmp(".", dirp->d_name, dirp->d_reclen) ||
				!strncmp("..", dirp->d_name, dirp->d_reclen)) continue;

		snprintf((char *)fl->src_file, sizeof(fl->src_file), "%s/%s", dir, dirp->d_name);
		fl++;
		if (++count > MAX_FILELIST_COUNT) break;
	}

	return file_list;
}

/************************************************/
/* Pattern Update - LUS													*/
/************************************************/

int Local_Pattern_Update_from_DIR (PROC_t *proc)
{
	DATA_t *data = proc->data;
	FILE_t *flist = &(data->file_list);

	int   apply_ret = 0;

	snprintf(proc->update_path, sizeof(proc->update_path)
			, "%s/%s", LIVEUPDATE_PATH, data->release_date);

	//TODO Check Type Code (Update Category)
	if (get_unzip_path (proc) < 0) {
		__AGENT_DBGP(__FAIL, "Worker: Unsupport Category\n");
		return STAT_NOT_SUPPORT_CODE_ERR;
	}

	//TODO Rule DB Unzip
	if (file_unzip ((char *)flist->src_file, proc->unzip_path) < 0) {
		__AGENT_DBGP(__FAIL, "Worker: DB File Unzip Fail\n");
		return STAT_FILE_UNZIP_ERR;
	}

	//TODO Help DB Unzip
	if (strlen (proc->help_path) && strlen (flist->help_file)) {
		if (file_unzip ((char *)flist->help_file, proc->help_path) < 0) {
			__AGENT_DBGP(__FAIL, "Worker: Help File Unzip Fail\n");
			return STAT_FILE_UNZIP_ERR;
		}
	}

	//TODO Apply
	apply_ret = pattern_update (proc);

	return apply_ret;
}

int pattern_update (PROC_t *proc)
{
	DATA_t *data = proc->data;
	FILE_t *flist = &(data->file_list);
	DBB_t  *dbb;

	struct dirent *item;
	DIR *dp = NULL;
	struct stat item_stat;

	int   rc, apply_ret = 0;

	proc->dbb = (DBB_t *) malloc (sizeof(DBB_t) * MAX_CATEGORY_CNT);	
	if (proc->dbb == NULL) {
		__AGENT_DBGP (__FAIL, "Worker: Fail to get memory allocation\n"); 

		apply_ret = STAT_MEMORY_ALLOC_ERR;
		goto pattern_update_error;	//...
	}

	dp = opendir(proc->unzip_path);
	if (dp == NULL) {
		__AGENT_DBGP (__FAIL, "Worker: Directory Open Fail(%s)\n", proc->unzip_path);
		if (proc->dbb) free (proc->dbb);

		apply_ret = STAT_FILE_PATH_ERR;
		goto pattern_update_error;	//...
	}

	//TODO Read Dir
	dbb	 = proc->dbb;
	while ((item = readdir(dp)) != NULL) {
		if (strcmp(item->d_name, "..") == 0 ||
				strcmp(item->d_name, ".") == 0 ) continue;

		memset (dbb, '\0', sizeof (DBB_t));
		snprintf (dbb->src_file, sizeof (dbb->src_file)
				, "%s/%s"
				, proc->unzip_path, item->d_name);

		memset(&item_stat, '\0', sizeof(struct stat));
		stat(dbb->src_file, &item_stat);
		if (S_ISDIR(item_stat.st_mode)) continue;
		dbb->src_size = item_stat.st_size;

		//TODO Support Check
		get_category_code (data->platform_code, item->d_name, dbb);
		if (dbb->index == -1) {
			__AGENT_DBGP (__FAIL
					, "Worker: Unsupported Category (%s)\n", item->d_name); 
			continue;	
		}
		dbb->Info = &Info_One30[dbb->index];

		dbb->update = 1;
		if (dbb->Info->category_number == ATTACK_AR_EIP) {
			snprintf (dbb->dst_file, sizeof(dbb->dst_file)
					, "%s/%s"
					, "../config", LUS_DB_RULESET_EIP3);	//dir info fix later
		} else if (dbb->Info->category_number == ATTACK_REPUTATION_BASE) {
			snprintf (dbb->dst_file, sizeof(dbb->dst_file)
					, "%s/%s"
					, "../config/reputation", item->d_name);	//dir info fix later
		} else {
			snprintf (dbb->dst_file, sizeof(dbb->dst_file)
					, "%s/%s"
					, "../config", item->d_name);	//dir info fix later
		}

		//TODO Hash Check
		rc = hash_check (dbb);
		if (rc > 0) {
			__AGENT_DBGP(__INFO
					, "Latest version(%s)\n", dbb->dst_file);
			continue; //latest version
		} else if (rc < 0) {
			__AGENT_DBGP(__INFO
					, "Fail to get hash(%s)\n", dbb->dst_file);
			continue;
		}

		//TODO Update Partial
		if (apply_ret = pattern_update_partial (dbb, data->release_date)) {

			proc->update_count++;

			__AGENT_DBGP(__INFO, "Worker: Rollback Start...\n");

			//TODO Rollback
			if ((rc = rollback_all (proc)) == 0)
				__AGENT_DBGP(__FAIL, "Worker: Rollback Fail\n");

			__AGENT_DBGP(__INFO, "Worker: Rollback Done\n");
			break;	
		} else {
			proc->update_count++;
			dbb++;
		}
	}
	
	if (proc->update_count == 0) goto pattern_update_error;

	//TODO Help
	if (strlen (proc->help_path) && strlen (flist->help_file)) {
		__AGENT_DBGP(__FAIL, "Worker: Help Update Start...\n");
		if (update_rulehelp (proc->help_path, data->lang_code) < 0) {
			if ((rc = rollback_all (proc)) == 0) 
				__AGENT_DBGP(__FAIL, "Rollback Fail\n");

			apply_ret = STAT_HELP_UPDATE_ERR;
			goto pattern_update_error;
		}
	}

	//TODO Hash Update
	if (hash_update (proc)) {
		__AGENT_DBGP(__INFO
				, "Worker: Update Release Info Fail(%s)\n", dbb->dst_file);
	}

	backup_clear (proc);

	if (dp) closedir(dp);
	if (proc->dbb) free (proc->dbb);

	return apply_ret;

pattern_update_error:

	if (dp) closedir(dp);
	if (proc->dbb) free (proc->dbb);

	return apply_ret;
}

int pattern_update_partial (DBB_t *dbb, char *release_date)
{
	int rc, ret = 0;

	CATEGORY_INFO_t	*info = dbb->Info;

	//TODO Backup
	snprintf (dbb->bak_file, sizeof (dbb->bak_file), "%s.bak", dbb->dst_file);

	if (access (dbb->dst_file, F_OK) == 0) {
		rc = file_copy (dbb->dst_file, dbb->bak_file);
	}

	//TODO Apply
	if (rule_apply_rms (dbb) < 0) {
		__AGENT_DBGP(__FAIL,"Worker: DB Update Fail\n");
		ret = STAT_DB_UPDATE_ERR;
		goto partial_update_error;
	}
	__AGENT_DBGP(__INFO,"Worker: DB Update Success\n");

	//TODO Compile
	if (compile_func (info->category_name, release_date)) {
		__AGENT_DBGP(__FAIL,"Worker: Compile(Loadlock) Fail\n");
		ret = STAT_COMPILE_ERR;
		goto partial_update_error;
	}
	__AGENT_DBGP(__INFO,"Worker: Compile(Loadlock) Success\n");

	return 0;

partial_update_error:

	return ret;
}

int get_unzip_path (PROC_t *proc)
{
	DATA_t *data = proc->data;
	char	 dir[64] = {0,};

	if (data->type_code[1] == '2') {	
		if (data->type_code[3] == '0') {
			snprintf (dir, sizeof (dir), "p");
			snprintf (proc->help_path, sizeof (proc->help_path)
					, "%s/%s", proc->update_path, "h");
		}
		else if (data->type_code[2] == '1') snprintf (dir, sizeof (dir), "r");
	}
	else if (data->type_code[1] == '4') {
				 if (data->type_code[2] == '3') snprintf (dir, sizeof (dir), "ar");
		else if (data->type_code[2] == '4') snprintf (dir, sizeof (dir), "ipr");
	}
	else {
		snprintf (dir, sizeof (dir), "tmp");
	}

	if (strlen (dir)) {
		snprintf (proc->unzip_path, sizeof (proc->unzip_path)
				, "%s/%s"
				, proc->update_path, dir);
		mkdir (proc->unzip_path, 0755);
	} else {
		return -1;
	}

	return 0;
}

void get_category_code (int platform_code, char *filename, DBB_t *dbb)
{

	if (strncmp (filename
				, LUS_DB_WEB_RULESET
				, strlen (LUS_DB_WEB_RULESET)) == 0) { 
		dbb->index = IDX_WEBCGI;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_webcgi;

	} else if (strncmp (filename
				, LUS_DB_USER_WEB_RULESET
				, strlen (LUS_DB_USER_WEB_RULESET)) == 0) { 
		dbb->index = IDX_WEBCGI_USER;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_webcgi;

	} else if (strncmp (filename
				, LUS_DB_ANOMALY_RULESET
				, strlen (LUS_DB_ANOMALY_RULESET)) == 0) { 
		dbb->index = IDX_ANOMALY;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_anomaly;

	} else if (strncmp (filename
				, LUS_DB_PB_RULESET
				, strlen (LUS_DB_PB_RULESET)) == 0) { 
		dbb->index = IDX_PATTERNBLOCK;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_patternblock;

	} else if (strncmp (filename
				, LUS_DB_USER_RULESET
				, strlen (LUS_DB_USER_RULESET)) == 0) { 
		dbb->index = IDX_USERDEFINE;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_patternblock;

	} else if (strncmp (filename
				, LUS_DB_HM_RULSET
				, strlen (LUS_DB_HM_RULSET)) == 0) { 
		dbb->index = IDX_HM;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_hm;

	} else if (strncmp (filename
				, LUS_DB_RULESET_EIP3
				, strlen (LUS_DB_RULESET_EIP3)) == 0) { 
		dbb->index = IDX_EIP3;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_eip3;

	} else if (strncmp (filename
				, LUS_DB_DNSURL_RULESET
				, strlen (LUS_DB_DNSURL_RULESET)) == 0) { 
		dbb->index = IDX_DNSURL;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_dnsurl;

	} else if (strncmp (filename
				, LUS_DB_AUTOSIG_RULESET
				, strlen (LUS_DB_AUTOSIG_RULESET)) == 0) { 
		dbb->index = IDX_AUTOSIG;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_autosig;	//ddx

	} else if (strncmp (filename
				, LUS_DB_SNORT_RULESET
				, strlen (LUS_DB_SNORT_RULESET)) == 0) { 
		dbb->index = IDX_SNORT_UPDATE;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_snort;

	} else if (strncmp (filename
				, LUS_DB_SNORT_WINS_RULESET
				, strlen (LUS_DB_SNORT_WINS_RULESET)) == 0) { 
		dbb->index = IDX_SNORT_WINS;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_snort;

	} else if (strncmp (filename
				, LUS_DB_SNORT_USER_RULESET
				, strlen (LUS_DB_SNORT_USER_RULESET)) == 0) { 
		dbb->index = IDX_SNORT_USER;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_snort;

	} else if (strncmp (filename
				, LUS_DB_YARA_RULESET
				, strlen (LUS_DB_YARA_RULESET)) == 0) { 
		dbb->index = IDX_YARA_UPDATE;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_yara;

	} else if (strncmp (filename
				, LUS_DB_YARA_WINS_RULESET
				, strlen (LUS_DB_YARA_WINS_RULESET)) == 0) { 
		dbb->index = IDX_YARA_WINS;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_yara;

	} else if (strncmp (filename
				, LUS_DB_YARA_USER_RULESET
				, strlen (LUS_DB_YARA_USER_RULESET)) == 0) { 
		dbb->index = IDX_YARA_USER;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_yara;

	} else if (strncmp (filename
				, LUS_DB_INTERNAL_RULESET
				, strlen (LUS_DB_INTERNAL_RULESET)) == 0) { 
		dbb->index = IDX_INNTERRULE;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_internal;

	} else if (strncmp (filename
				, FILE_NAME_IPPOOL_RULESET
				, strlen (FILE_NAME_IPPOOL_RULESET)) == 0) { 
		dbb->index = IDX_IPPOOL;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ippool;

	} else if (strncmp (filename
				, FILE_NAME_VIPS_RULESET
				, strlen (FILE_NAME_VIPS_RULESET)) == 0) { 
		dbb->index = IDX_VIPS3;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_vips;

	} else if (strncmp (filename
				, FILE_NAME_VIPS_ANOMALY
				, strlen (FILE_NAME_VIPS_ANOMALY)) == 0) { 
		dbb->index = IDX_VIPS_ANOMALY;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_vips_anomaly;

	} else if (strncmp (filename
				, LUS_DB_NQOS_RULESET
				, strlen (LUS_DB_NQOS_RULESET)) == 0) { 
		dbb->index = IDX_NQOS;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_nqos;

	} else if (strncmp (filename
				, LUS_DB_NQOS_APPS_GRP
				, strlen (LUS_DB_NQOS_APPS_GRP)) == 0) { 
		dbb->index = IDX_NQOS;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_nqos_grp;

	} else if (strncmp (filename
				, LUS_DB_AR_POLICY
				, strlen (LUS_DB_AR_POLICY)) == 0) { 
		dbb->index = IDX_AR_POLICY;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ar_policy;

	} else if (strncmp (filename
				, LUS_DB_RULESET_EIP3_AR
				, strlen (LUS_DB_RULESET_EIP3_AR)) == 0) { 
		dbb->index = IDX_AR_EIP;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ar_eip3;

	} else if (strncmp (filename
				, FILE_NAME_AR_PATTERN
				, strlen (FILE_NAME_AR_PATTERN)) == 0) { 
		dbb->index = IDX_AR_PATTERN;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ar_pattern;

	} else if (strncmp (filename
				, LUS_DB_IPR_POLICY
				, strlen (LUS_DB_IPR_POLICY)) == 0) { 
		dbb->index = IDX_IPR_POLICY;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ipr_policy;

	} else if (strncmp (filename + IPR_CODE_GAP
				, FILE_NAME_IPR_PATTERN
				, strlen (FILE_NAME_IPR_PATTERN)) == 0) { 
		dbb->index = IDX_IPR_PATTERN;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ipr_pattern;

	} else if (strncmp (filename
				, FILE_NAME_IPR_CATEGORY
				, strlen (FILE_NAME_IPR_CATEGORY)) == 0) { 
		dbb->index = IDX_IPR_CATEGORY;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ipr_pattern;

	} else if (strncmp (filename
				, FILE_NAME_IPR_SCHEDULE
				, strlen (FILE_NAME_IPR_SCHEDULE)) == 0) { 
		dbb->index = IDX_IPR_SCHEDULE;
		if(platform_code == ONE_3_X)
			dbb->apply_func = rule_apply_one30_ipr_pattern;

	} else {
		dbb->index = -1;

	}

	return ;
}

int rule_apply_rms (DBB_t *dbb)
{
	CATEGORY_INFO_t	*info = dbb->Info;

	if(access (dbb->src_file, R_OK) != 0) { 
		__AGENT_DBGP (__INFO, "No File Exists (%s)\n", dbb->src_file);
		return -1;
	}

	if (access (dbb->dst_file, R_OK) != 0) 
		createTable_category (dbb->dst_file, info->db_table_index);

	if (dbb->src_size == 0) {
		__AGENT_DBGP (__INFO, "File Size is Zero (%s)\n", dbb->src_file);
		return 0;
	}

	if (dbb->apply_func == NULL) return -1;

	return dbb->apply_func (dbb->dst_file, dbb->src_file, info->category_number);
}

/***********************************************/
/*LUS에서 보내주는 DB List 						         */
/*1. anomaly.dbb 						 									 */
/*2. hmruleset.dbb														 */
/*3. internelruleset.dbb											 */
/*4. patternblock.dbb													 */
/*5. rule_eip3.dbb / rule_eip2.dbb						 */
/*6. user_patternblock.dbb										 */
/*7. user_snortruleset.dbb										 */
/*8. user_webcgi.dbb													 */
/*9. userdefine.dbb														 */
/*10. webcgi.dbb															 */
/*=============================================*/
/* SNIPER_ONE과 SNIPER_R3의 DB차이점은         */
/* license_flag의 유뮤 												 */
/***********************************************/

int rule_apply_one30_internal (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			"	REPLACE INTO master 	    "
			"	(                     	  "
			"				 category,          "
			"				 code,              "
			"				 name,              "
			"				 filter,            "
			"				 nlimit,            "
			"				 expire,            "
			"				 risk,              "
			"				 alarm,             "
			"				 detecting,         "
			"				 blocking,          "
			"				 blackhole,         "
			"				 raw,               "
			"				 ipcnt,             "
			"				 overflow,          "
			"				 method,            "
			"				 abstract1,         "
			"				 abstract2,         "
			"				 by_study,          "
			"				 v6abstract1,       "
			"				 v6abstract2,       "
			"				 license_flag       "
			"	 )                        "
			"		SELECT                  "
			"        uirm.category,     "
			"				 uirm.code,         "
			"			 	 uirm.name,         "
			"			 	 uirm.filter,       "
			"	       uirm.nlimit,       "
			"	       uirm.expire,       "
			"	       uirm.risk,         "
			"	       uirm.alarm,        "
			"	       uirm.detecting,    "
			"	       uirm.blocking,     "
			"	       uirm.blackhole,    "
			"	       uirm.raw,          "
			"	       uirm.ipcnt,        "
			"	       uirm.overflow,     "
			"	       uirm.method,       "
			"	       uirm.abstract1,    "
			"	       uirm.abstract2,		"
			"      	 uirm.by_study,     "
			"        uirm.v6abstract1,  "
			"        uirm.v6abstract2,  "
			"				 uirm.license_flag  "
			" from uir.master as 'uirm', master as 'orig' where "
			" orig.category = uirm.category and "
			"	orig.code = uirm.code;  "
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_webcgi (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			"       code        "
			"       ,filter      "
			"       ,nlimit      "
			"       ,expire      "
			"       ,risk        "
			"       ,alarm       "
			"			  ,detecting   "
			"			  ,blocking    "
			"			  ,blackhole   "
			"			  ,raw         "
			"			  ,ipcnt       "
			"			  ,ndelete     "
			"			  ,stype       "
			"			  ,nupdate     "
			"			  ,author      "
			"			  ,method      "
			"			  ,nocase      "
			"			  ,abstract1   "
			"			  ,abstract2   "
			"			  ,pattern     "
			"			  ,name        "
			"			  ,v6abstract1 "
			"			  ,v6abstract2 "
			"			  ,license_flag"
			"	 )          "
			"	SELECT 						"
			"				uirm.code 			  "
			"				,uirm.filter      "
			"				,uirm.nlimit      "
			"				,uirm.expire      "
			"				,uirm.risk        "
			"				,uirm.alarm       "
			"				,uirm.detecting   "
			"				,uirm.blocking    "
			"				,uirm.blackhole   "
			"				,uirm.raw         "
			"				,uirm.ipcnt       "
			"				,uirm.ndelete     "
			"				,uirm.stype       "
			"				,uirm.nupdate     "
			"				,uirm.author      "
			"				,uirm.method      "
			"				,uirm.nocase      "
			"				,uirm.abstract1   "
			"				,uirm.abstract2   "
			"				,uirm.pattern     "
			"				,uirm.name        "
			"				,uirm.v6abstract1 "
			"				,uirm.v6abstract2 "
			"				,uirm.license_flag"
			" from uir.master as uirm;"
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	if (category == ATTACK_WEBCGI_USERDEFINE) return 0;

	//more
	memset (sql, '\0', sizeof (sql));
	snprintf(sql, sizeof(sql),
			" UPDATE master           "
			" SET license_flag = %d;  "
			, LIC_FLAG_IPS_PATTERN);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL
				, "DB License Update Fail (%d)\n", category);
		return -1;
	}

	return 0;
}

int rule_apply_one30_anomaly (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" category					"
			" ,code							"
			" ,name							"
			" ,bound_idx				"
			" ,protocol					"
			" ,port							"
			" ,pps_flag					"
			" ,nlimit						"
			" ,nlimit_max				"
			" ,nlimit_avg				"
			" ,nlimit_min				"
			" ,risk							"
			" ,alarm						"
			" ,detecting				"
			" ,raw							"
			" ,by_study					"
			" ,detection_flag		"
			" ,ndelete          "
			"	 )                "
			"	SELECT 						"
			" uirm.category      "
			" ,uirm.code         "
			" ,uirm.name         "
			" ,uirm.bound_idx    "
			" ,uirm.protocol     "
			" ,uirm.port         "
			" ,uirm.pps_flag     "
			" ,uirm.nlimit       "
			" ,uirm.nlimit_max   "
			" ,uirm.nlimit_avg   "
			" ,uirm.nlimit_min   "
			" ,uirm.risk         "
			" ,uirm.alarm        "
			" ,uirm.detecting    "
			" ,uirm.raw           "
			" ,uirm.by_study      "
			" ,uirm.detection_flag"
			" ,uirm.ndelete       "
			" from uir.master as uirm;" //where
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_patternblock (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf (sql, sizeof(sql),
			"	ATTACH '%s' as uir;"
			" DELETE from master;"
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" 			 code,          "
			"        filter,        "
			"        nlimit,        "
			"        expire,        "
			"        risk,          "
			"        alarm,         "
			"        detecting,     "
			"        blocking,      "
			"        blackhole,     "
			"        raw,           "
			"				 ipcnt,         "
			"        ndelete,       "
			"        stype,         "
			"        nupdate,       "
			"        method,        "
			"        nocase,        "
			"        abstract1,     "
			"        abstract2,     "
			"        author,        "
			"        pattern,       "
			"        name,          "
			"        offset_flag,   "
			"        reqrsp,        "
			"        offset,        "
			"        protocol,      "
			"        sport,         "
			"        by_study,      "
			"        v6abstract1,   "
			"        v6abstract2,   "
			"        license_flag   "
			"	 )                         "
			"		SELECT							     "
			" 			 uirm.code,          "
			"        uirm.filter,        "
			"        uirm.nlimit,        "
			"        uirm.expire,        "
			"        uirm.risk,          "
			"        uirm.alarm,         "
			"        uirm.detecting,     "
			"        uirm.blocking,      "
			"        uirm.blackhole,     "
			"        uirm.raw,           "
			"        uirm.ipcnt,         "//기존 꺼 유지 
			"        uirm.ndelete,       "
			"        uirm.stype,         "
			"        uirm.nupdate,       "
			"        uirm.method,        "
			"        uirm.nocase,        "
			"        uirm.abstract1,     "
			"        uirm.abstract2,     "
			"        uirm.author,        "
			"        uirm.pattern,       "
			"        uirm.name,          "
			"        uirm.offset_flag,   "
			"        uirm.reqrsp,        "
			"        uirm.offset,        "
			"        uirm.protocol,      "
			"        uirm.sport,         "
			"        uirm.by_study,      "//기존 꺼 유지 
			"        uirm.v6abstract1,   "
			"        uirm.v6abstract2,   "
			"        uirm.license_flag   "
			" from uir.master as uirm;"//where "
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	if (category == ATTACK_USERDEFINE) return 0;

	//more
	memset (sql, '\0', sizeof (sql));
	snprintf(sql, sizeof(sql),
			" UPDATE master           "
			" SET license_flag = %d;  "
			, LIC_FLAG_IPS_PATTERN);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL
				, "DB License Update Fail (%d)\n", category);
		return -1;
	}

	return 0;
}

int rule_apply_one30_hm (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" code          " 
			" ,filter       "
			" ,nlimit       "
			" ,expire       "
			" ,risk         "
			" ,alarm        "
			" ,detecting    "
			" ,blocking     "
			" ,blackhole    "
			" ,raw          "
			" ,ipcnt        "
			" ,ndelete      "
			" ,nupdate      "
			" ,reqrsp       "
			" ,method       "
			" ,pattern      "
			" ,mask					"
			" ,name         "
			"	 )                "
			"	SELECT 						"
			" uirm.code           "
			" ,uirm.filter        "
			" ,uirm.nlimit        "
			" ,uirm.expire        "
			" ,uirm.risk          "
			" ,uirm.alarm         "
			" ,uirm.detecting     "
			" ,uirm.blocking      "
			" ,uirm.blackhole     "
			" ,uirm.raw           "
			" ,uirm.ipcnt         "
			" ,uirm.ndelete       "
			" ,uirm.nupdate       "
			" ,uirm.reqrsp        "
			" ,uirm.method        "
			" ,uirm.pattern       "
			" ,uirm.mask					"
			" ,uirm.name          "
			" from uir.master as uirm;" //where "
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_eip3 (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" category       "
			" ,code          "
			" ,flag          "
			" ,proto				 "
			" ,s_port				 "
			" ,d_port				 "
			" ,s_network     "
			" ,s_netmask     "
			" ,s_broadcast   "
			" ,d_network     "
			" ,d_netmask     "
			" ,d_broadcast   "
			" ,v_id					 "
			"	 )          "
			"	SELECT 						"
			" uirm.category      "
			" ,uirm.code         "
			" ,uirm.flag         "
			" ,uirm.proto				 "
			" ,uirm.s_port			 "
			" ,uirm.d_port			 "
			" ,uirm.s_network    "
			" ,uirm.s_netmask    "
			" ,uirm.s_broadcast  "
			" ,uirm.d_network    "
			" ,uirm.d_netmask    "
			" ,uirm.d_broadcast  "
			" ,uirm.v_id				 "
			" from uir.master as uirm;" //where "
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_dnsurl (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			"	        code,           "
			"	        filter,         "
			"	        nlimit,         "
			"	        expire,         "
			"	        risk,           "
			"	        alarm,          "
			"	        detecting,      "
			"	        blocking,       "
			"	        blackhole,      "
			"	        raw,            "
			"	        ndelete,        "
			"	        ipcnt,          "
			"	        stype,          "
			"	        pkt_len1,       "
			"	        pkt_len2,       "
			"	        nupdate,        "
			"	        reqrsp,         "
			"	        nocase,         "
			"	        method,         "
			"	        abstract1,      "
			"	        abstract2,      "
			"	        offset,         "
			"	        offset_flag,    "
			"	        dns_querytype,  "
			"	        pattern,        "
			"	        name,           "
			"         by_study,       "
			"         v6abstract1,    "
			"         v6abstract2     "
			"	)                       "
			"	SELECT 					      			"
			"	         uirm.code           "
			"	        ,uirm.filter         "
			"	        ,uirm.nlimit         "
			"	        ,uirm.expire         "
			"	        ,uirm.risk           "
			"	        ,uirm.alarm          "
			"	        ,uirm.detecting      "
			"	        ,uirm.blocking       "
			"	        ,uirm.blackhole      "
			"	        ,uirm.raw            "
			"	        ,uirm.ndelete        "
			"	        ,uirm.ipcnt          "
			"	        ,uirm.stype          "
			"	        ,uirm.pkt_len1       "
			"	        ,uirm.pkt_len2       "
			"	        ,uirm.nupdate        "
			"	        ,uirm.reqrsp         "
			"	        ,uirm.nocase         "
			"	        ,uirm.method         "
			"	        ,uirm.abstract1      "
			"	        ,uirm.abstract2      "
			"	        ,uirm.offset         "
			"	        ,uirm.offset_flag    "
			"	        ,uirm.dns_querytype  "
			"	        ,uirm.pattern        "
			"	        ,uirm.name           "
			"         ,uirm.by_study       "
			"         ,uirm.v6abstract1    "
			"         ,uirm.v6abstract2     "
			" from uir.master as uirm;" //where "
			" detach uir;          		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_autosig (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; 		"	
			"	REPLACE INTO master 	  "
			"	(                     	"
			"        code,           	"
			"        stype,         	"
			"        protocol,      	"
			"        detecting,     	"
			"        blocking,      	"
			"        alarm,         	"
			"        risk,          	"
			"        raw,           	"
			"        ipcnt,         	"
			"        ekey_cnt,      	"
			"        min_pps,       	"
			"        nupdate,       	"
			"        name           	"
			"	 )                    	"
			"	SELECT 									"
			"				 uirm.code,       "
			"        uirm.stype,      "
			"        uirm.protocol,   "
			"        uirm.detecting,  "
			"        uirm.blocking,   "
			"        uirm.alarm,      "
			"        uirm.risk,       "
			"        uirm.raw,        "
			"        uirm.ipcnt,    	"
			"        uirm.ekey_cnt,   "
			"        uirm.min_pps,    "
			"        uirm.nupdate,    "
			"        uirm.name        "
			" from uir.master as uirm;"
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_snort (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE FROM master;"
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" code          " 
			" ,filter       "
			" ,nlimit       "
			" ,expire       "
			" ,risk         "
			" ,alarm        "
			" ,detecting    "
			" ,blocking     "
			" ,blackhole    "
			" ,raw          "
			" ,ipcnt        "
			" ,ndelete      "
			" ,stype        "
			" ,nupdate      "
			" ,author       "
			" ,method       "
			" ,nocase       "
			" ,abstract1    "
			" ,abstract2    "
			" ,name         "
			" ,offset_flag  "
			" ,reqrsp       "
			" ,offset       " 
			" ,protocol     "
			" ,sport        "
			" ,ncscrule     "

			"	 )                        "
			"		SELECT                  "
			"  uirm.code          "
			" ,uirm.filter        "
			" ,uirm.nlimit        "
			" ,uirm.expire        "
			" ,uirm.risk          "
			" ,uirm.alarm         "
			" ,uirm.detecting     "
			" ,uirm.blocking      "
			" ,uirm.blackhole     "
			" ,uirm.raw           "
			" ,uirm.ipcnt         "
			" ,uirm.ndelete       "
			" ,uirm.stype         "
			" ,uirm.nupdate       "
			" ,uirm.author        "
			" ,uirm.method        "
			" ,uirm.nocase        "
			" ,uirm.abstract1     "
			" ,uirm.abstract2     "
			" ,uirm.name          "
			" ,uirm.offset_flag   "
			" ,uirm.reqrsp        "
			" ,uirm.offset        " 
			" ,uirm.protocol      "
			" ,uirm.sport         "
			" ,uirm.ncscrule      "
			" from uir.master as 'uirm';"
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_yara (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir;"
			" DELETE FROM master;"
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" code          						" 
			" ,detecting    						"
			" ,ndelete      						"
			" ,name       							"
			" ,yaraname       					"
			" ,yararule       					"
			"	 )                        "
			"		SELECT                  "
			" uirm.code          				"	 
			" ,uirm.detecting    				"
			" ,uirm.ndelete      				"
			" ,uirm.name       					"
			" ,uirm.yaraname       			"
			" ,uirm.yararule       			"
			" from uir.master as 'uirm';"
			" detach uir;         			"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_ippool (char *dst, char *src, int category)
{
	int  rc = 0;

	if (access (dst, F_OK) == 0) unlink (dst);

	rc = file_copy (src, dst);

	if (access (dst, F_OK) != 0) rc = -1;

	return rc;
}

int rule_apply_one30_vips (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	char eip_db[512] = {0,};

	snprintf (eip_db, sizeof(eip_db), "../config/%s", LUS_DB_RULESET_EIP3);

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" category   			    			"
			" ,code          						"
			" ,filter        						"
			" ,detecting				 				"
			" ,blocking				 					"
			" ,blackhole				 				"
			" ,alarm     								"
			" ,by_study     						"
			" ,v_id          						"
			" ,nlimit				 						"
			" ,expire				 						"
			" ,ipcnt     								"
			" ,raw					 						"
			"	 )          							"
			"	SELECT 										"
			"  uirm.category       			"
			" ,uirm.code         				"
			" ,uirm.filter       				"
			" ,uirm.detecting		 				"
			" ,uirm.blocking						"
			" ,uirm.blackhole		 				"
			" ,uirm.alarm     					"
			" ,uirm.by_study     				"
			" ,uirm.v_id         				"
			" ,uirm.nlimit							"
			" ,uirm.expire							"
			" ,uirm.ipcnt     					"
			" ,uirm.raw					 				"
			" from uir.master as uirm;	" //where "
/*
			"	ATTACH '%s' as eruledb;           								"
			"	DELETE FROM erule_cnt;            								"
			"	INSERT OR REPLACE INTO erule_cnt									"
			" (category, code, v_id, eipcnt)     								"
			"	SELECT																						"
			" edb.category, edb.code, edb.v_id, count(edb.v_id)	"
			"	FROM eruledb.master as edb         								"
			"	WHERE edb.v_id != '65535'      										"
			"	GROUP BY edb.category, edb.code, edb.v_id; 				"
			" detach eruledb;         	"
*/
			" detach uir;         			"
			, src);
//			, eip_db);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_vips_anomaly (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(                     	  "
			" category   			    			"
			" ,code          						"
			" ,v_id          						"
			" ,detecting				 				"
			" ,nlimit				 						"
			" ,nlimit_max     					"
			" ,nlimit_avg     					"
			" ,nlimit_min   						"
			" ,alarm     								"
			" ,by_study     						"
			" ,detection_flag   				"
			" ,raw					 						"
			"	 )          							"
			"	SELECT 										"
			"  uirm.category  			    "
			" ,uirm.code          			"
			" ,uirm.v_id          			"
			" ,uirm.detecting					 	"
			" ,uirm.nlimit				 			"
			" ,uirm.nlimit_max   	  		"
			" ,uirm.nlimit_avg   	  		"
			" ,uirm.nlimit_min   				"
			" ,uirm.alarm     					"
			" ,uirm.by_study     				"
			" ,uirm.detection_flag	   	"
			" ,uirm.raw					 				"
			" from uir.master as uirm;	" //where "
			" detach uir;         		"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_nqos (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; 						"
			" DELETE from master; 						"
			"	INSERT OR REPLACE INTO master		"
			"	(                     	  			"
			" priority   			    						"
			" ,nupdate          							"
			" ,interface          						"
			" ,protocol				 								"
			" ,o_id_s				 									"
			" ,o_id_d				 									"
			" ,sip     												"
			" ,smask     											"
			" ,dip     												"
			" ,dmask     											"
			" ,sport   												"
			" ,dport   												"
			" ,option     										"
			" ,nlimit     										"
			" ,type   												"
			" ,desc					 									"
			" ,etc     												"
			" ,code     											"
			" ,use     												"
			" ,alarm   												"
			" ,ndelete   											"
			" ,detecting     									"
			" ,blocking     									"
			" ,ipv   													"
			"	 )          										"
			"	SELECT 													"
			" uirm.priority   		 						"
			" ,uirm.nupdate          					"
			" ,uirm.interface          				"
			" ,uirm.protocol				 					"
			" ,uirm.o_id_s				 						"
			" ,uirm.o_id_d				 						"
			" ,uirm.sip     									"
			" ,uirm.smask     								"
			" ,uirm.dip     									"
			" ,uirm.dmask     								"
			" ,uirm.sport   									"
			" ,uirm.dport   									"
			" ,uirm.option     								"
			" ,uirm.nlimit     								"
			" ,uirm.type   										"
			" ,uirm.desc					 						"
			" ,uirm.etc     									"
			" ,uirm.code     									"
			" ,uirm.use     									"
			" ,uirm.alarm   									"
			" ,uirm.ndelete   								"
			" ,uirm.detecting     						"
			" ,uirm.blocking     							"
			" ,uirm.ipv   										"
			" from uir.master as uirm;				" //where "
			" detach uir;         						"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_nqos_grp (char *dst, char *src, int category)
{
	int  rc = 0;

	if (access (dst, F_OK) == 0) unlink (dst);

	rc = file_copy (src, dst);

	if (access (dst, F_OK) != 0) rc = -1;

	return rc;
}

int rule_apply_one30_ar_policy (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; 						"
			" DELETE from ar_control_rule; 						"
			"	INSERT OR REPLACE INTO ar_control_rule"
			"	(                     	  			"
			"  code     											"
			" ,detecting     									"
			" ,control          							"
			" ,ipcnt		          						"
			"	 )          										"
			"	SELECT 													"
			"  uirm.code     									"
			" ,uirm.detecting     						"
			" ,uirm.control          					"
			" ,uirm.ipcnt		          				"
			" from uir.ar_control_rule as uirm;				"
			" detach uir;         						"
			, src);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_ar_eip3 (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[2048]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; "
			" DELETE from master; "
			"	INSERT OR REPLACE INTO master"
			"	(              "
			" category       "
			" ,code          "
			" ,flag          "
			" ,proto				 "
			" ,s_port				 "
			" ,d_port				 "
			" ,s_network     "
			" ,s_netmask     "
			" ,s_broadcast   "
			" ,d_network     "
			" ,d_netmask     "
			" ,d_broadcast   "
			" ,v_id					 "
			"	 )          			 "
			"	SELECT 						 "
			" uirm.category      "
			" ,uirm.code         "
			" ,uirm.flag         "
			" ,uirm.proto				 "
			" ,uirm.s_port			 "
			" ,uirm.d_port			 "
			" ,uirm.s_network    "
			" ,uirm.s_netmask    "
			" ,uirm.s_broadcast  "
			" ,uirm.d_network    "
			" ,uirm.d_netmask    "
			" ,uirm.d_broadcast  "
			" ,uirm.v_id				 "
			" from uir.master as uirm"
			" where uirm.category = %d;"
			" detach uir;         		"
			, src, ATTACK_AR_CTL);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

int rule_apply_one30_ar_pattern (char *dst, char *src, int category)
{
	int  rc = 0;

	if (access (dst, F_OK) == 0) unlink (dst);

	rc = file_copy (src, dst);

	if (access (dst, F_OK) != 0) rc = -1;

	return rc;
}

int rule_apply_one30_ipr_policy (char *dst, char *src, int category)
{
	int  rc = SQLITE_OK;
	char sql[4096]={0,};

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' as uir; 											"

			" DELETE from reputation_policy; 						"
			"	INSERT OR REPLACE INTO reputation_policy	"
			"	(                     	  								"
			"  policy_id     														"
			" ,type_id     															"
			" ,cate_id          												"
			" ,rule_id		          										"
			" ,pool_id		          										"
			" ,desc		          												"
			" ,score		          											"
			" ,policy		          											"
			" ,filter		          											"
			" ,priority		          										"
			" ,risk		          												"
			" ,alert		          											"
			" ,raw		          												"
			" ,note		          												"
			" ,schedule_id		          								"
			"	 )          															"
			"	SELECT 																		"
			"  uirm.policy_id     											"
			" ,uirm.type_id     												"
			" ,uirm.cate_id          										"
			" ,uirm.rule_id		          								"
			" ,uirm.pool_id		          								"
			" ,uirm.desc		          									"
			" ,uirm.score		          									"
			" ,uirm.policy		          								"
			" ,uirm.filter		          								"
			" ,uirm.priority		          							"
			" ,uirm.risk		          									"
			" ,uirm.alert		          									"
			" ,uirm.raw		          										"
			" ,uirm.note		          									"
			" ,uirm.schedule_id		         							"
			" from uir.reputation_policy as uirm;				"

			" DELETE from reputation_network;						"
			"	INSERT OR REPLACE INTO reputation_network	"
			"	(                     	  								"
			"  policy_id     														"
			" ,score		          											"
			" ,ver     																	"
			" ,naddr          													"
			" ,cidr		          												"
			"	 )          															"
			"	SELECT 																		"
			"  uirm.policy_id     											"
			" ,uirm.score		          									"
			" ,uirm.ver     														"
			" ,uirm.naddr          											"
			" ,uirm.cidr		          									"
			" from uir.reputation_network as uirm;			"
	
			" DELETE from reputation_url;								"
			"	INSERT OR REPLACE INTO reputation_url			"
			"	(                     	  								"
			"  policy_id     														"
			" ,score		          											"
			" ,domain  																	"
			" ,uri    	      													"
			"	 )          															"
			"	SELECT 																		"
			"  uirm.policy_id     											"
			" ,uirm.score		          									"
			" ,uirm.domain   														"
			" ,uirm.uri	          											"
			" from uir.reputation_url as uirm;					"

			" DETACH uir;         											"
			, src);
	fprintf (stderr, "SQL: %s\n", sql);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Update Fail (%s)\n", dst);
		return -1;
	}

	return 0;
}

//copy only
int rule_apply_one30_ipr_pattern (char *dst, char *src, int category)
{
	int  rc = 0;

	if (access (dst, F_OK) == 0) unlink (dst);

	rc = file_copy (src, dst);

	if (access (dst, F_OK) != 0) rc = -1;

	return rc;
}

int createTable_rulehelp (char *db, char *table, char *index)
{
	int rc = SQLITE_OK;
	char sql[8192];

	memset (sql, '\0', sizeof(sql));

	snprintf (sql, sizeof (sql),
			"CREATE TABLE %s ("
			"  nupdate int"
			", category int NOT NULL DEFAULT 0"
			", att_code int"
			", att_name varchar(256)"
			", risk int NOT NULL DEFAULT 3"
			", att_class varchar(256)"
			", vul_system text"
			", att_exp text"
			", att_affect text"
			", solution text"
			", potential text"
			", spoof int"
			", cve varchar(32)"
			", ref_site1 varchar(1024)"
			", ref_site2 varchar(1024)"
			", ref_site3 varchar(1024)"
			", ref_site4 varchar(1024)"
			", ref_site5 varchar(1024));"
			"CREATE UNIQUE INDEX %s ON %s(category, att_code);"
			, table
			, index, table);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "Worker: Help DB Create Fail\n");
		return -1;
	}
	__AGENT_DBGP (__INFO, "Worker: Help DB Create Success\n");

	return 0;
}

int update_rulehelp (char *help_path, int lang_code)
{
	char sql[2048] = {0,};
	char dst[512] = {0,};
	char src[512] = {0,};
	char bak[512] = {0,};
	int  rc = SQLITE_OK;

	char table [64];
	char index [64];

	if (lang_code == USNIPER_US_CODE ) {
		snprintf(table, sizeof(table), "%s", "ENG_HELP");
		snprintf(index, sizeof(index), "%s", "eng_code_key");
	} else if (lang_code == USNIPER_KOREA_CODE ) {
		snprintf(table, sizeof(table), "%s", "KOR_HELP");
		snprintf(index, sizeof(index), "%s", "kor_code_key");
	} else if (lang_code == USNIPER_JAPAN_CODE ) {
		snprintf(table, sizeof(table), "%s", "JPN_HELP");
		snprintf(index, sizeof(index), "%s", "jpn_code_key");
	} else {
		snprintf(table, sizeof(table), "%s", "KOR_HELP");
		snprintf(index, sizeof(index), "%s", "kor_code_key");
	}

	//TODO Help DB (../liveupdate/yyyy.../h/rulehelp.dbb)
	snprintf(src, sizeof(src)
			, "%s/%s", help_path, LUS_DB_HELP_RULESET);
	if (access (src, R_OK) != 0) return -1;

	strcpy (dst, DB_HELPSET);
	if (access (dst, R_OK) != 0) {
		if (createTable_rulehelp (dst, table, index)) return -1;
	} else {
		//TODO Backup
		snprintf (bak, sizeof (bak), "%s.bak", dst);
		file_copy (dst, bak);
	}

	snprintf(sql, sizeof(sql),
			"	ATTACH '%s' AS uir; "
			"	INSERT or REPLACE INTO %s "
			"	(                     	  "
			"        category,      "
			"        att_code,      "
			"        att_name,      "
			"        risk,          "
			"        att_class,     "
			"        vul_system,    "
			"        att_exp,       "
			"        att_affect,    "
			"        solution,      "
			"        potential,     "
			"        spoof,         "
			"        cve,           "
			"        ref_site1,     "
			"        ref_site2,     "
			"        ref_site3,     "
			"        ref_site4,     "
			"        ref_site5      "
			"	 )                         "
			"		SELECT                   "
			"        uirm.category,      "
			"        uirm.att_code,      "
			"        uirm.att_name,      "
			"        uirm.risk,          "
			"        uirm.att_class,     "
			"        uirm.vul_system,    "
			"        uirm.att_exp,       "
			"        uirm.att_affect,    "
			"        uirm.solution,      "
			"        uirm.potential,     "
			"        uirm.spoof,         "
			"        uirm.cve,           "
			"        uirm.ref_site1,     "
			"        uirm.ref_site2,     "
			"        uirm.ref_site3,     "
			"        uirm.ref_site4,     "
			"        uirm.ref_site5      "
			" FROM uir.%s AS 'uirm'; 		 "
			" DETACH uir;         			 "
			, src
			, table
			, table);

	rc = ExecDBQuery (dst, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "Help DB Update Fail\n");

		if (access (dst, F_OK) == 0) unlink (dst);
		rc = file_move (dst, bak);
		__AGENT_DBGP (__INFO, "Rollback %s (%d)\n", dst, rc);
		if (access (dst, F_OK) == 0) unlink (bak);

		return -1;
	}

	__AGENT_DBGP (__INFO, "Worker: Help DB Update Success\n");

	return 0;
}

int createTable_release (char *db)
{
	char sql[8192];

	memset (sql, '\0', sizeof(sql));

	snprintf (sql, sizeof (sql),
			"CREATE TABLE master ("
			" update_date             TEXT\n"
			",release_date            TEXT\n"
			",category                 INT\n"
			",filename                TEXT\n"
			",filesize                 INT\n"
			",filehash                TEXT\n"
			");"
			"CREATE UNIQUE INDEX _key_master ON master(category);"
			);

	if (ExecDBQuery (db, sql)) return -1;

	return 0;
}

static void get_hash (DBB_t *dbb)
{
	FILE *fp;
	char *buf;
	long filesize;
	size_t len;

	if ((fp = fopen (dbb->src_file, "rb")) == NULL) return ;

	fseek (fp, 0, SEEK_END);
	filesize = ftell (fp);	
	fseek (fp, 0, SEEK_SET);

	buf = (char *) malloc (filesize);
	if(buf == NULL) {
		fclose (fp);
		return ;
	}
	memset (buf, '\0', filesize);

	len = fread (buf, sizeof (char), filesize, fp);

	strcpy(dbb->src_hash
			, (char *)sha2_stringSigCreate_hash_key 
				(buf, len));

	if (fp) fclose (fp);
	if (buf) free (buf);
}

int hash_check (DBB_t *dbb)
{
	char sql[8192];
	int  nRow = 0;

	CATEGORY_INFO_t	*info = dbb->Info;

	get_hash (dbb);
	if (strlen (dbb->src_hash) == 0) return -1;

	if (access (PATTERN_RELEASE_DB, R_OK) != 0) 
		createTable_release (PATTERN_RELEASE_DB);

	memset (sql, '\0', sizeof(sql));
	snprintf (sql, sizeof (sql),
			" SELECT * FROM master"
			" WHERE category=%d AND filehash='%s';"
			, info->category_number, dbb->src_hash);

	nRow = GetDBQueryCount (PATTERN_RELEASE_DB, sql);

	return nRow;
}

int hash_update (PROC_t *proc)
{
	DATA_t *data = proc->data;
	DBB_t  *dbb = proc->dbb;
	CATEGORY_INFO_t	*info;

	char sql[8192];
	int  i;

	if (access (PATTERN_RELEASE_DB, R_OK) != 0) 
		createTable_release (PATTERN_RELEASE_DB);

	for (i = 0; i < proc->update_count; i++) {
		if (!dbb->update) continue;
		info = dbb->Info;

		memset (sql, '\0', sizeof(sql));
		snprintf (sql, sizeof (sql),
				" INSERT OR REPLACE INTO master"
				"("
				" update_date"
				",release_date"
				",category"
				",filename"
				",filesize"
				",filehash"
				") VALUES ( "
				"'%s', '%s', '%d', '%s', '%d', '%s'"
				");"
				, data->update_date
				, data->release_date
				, info->category_number
				, dbb->src_file
				, dbb->src_size
				, dbb->src_hash);

		if (ExecDBQuery (PATTERN_RELEASE_DB, sql)) return -1;

		dbb++;
	}

	return 0;
}

int rollback_all (PROC_t *proc)
{
	int i, rc = 0;
	DBB_t  *dbb;

	int count = proc->update_count;

	dbb	 = proc->dbb;
	for (i = 0; i < proc->update_count; i++) {

		if (access (dbb->bak_file, F_OK) != 0) {
			__AGENT_DBGP (__INFO, "No backup File Exists (%s)\n", dbb->bak_file);
		} else {
			if (access (dbb->dst_file, F_OK) == 0) unlink (dbb->dst_file);

			rc = file_move (dbb->dst_file, dbb->bak_file);
			__AGENT_DBGP (__INFO
					, "Rollback %s (%d:%d)\n"
					, dbb->dst_file, count, rc);

			if (access (dbb->dst_file, F_OK) == 0) unlink (dbb->bak_file);
			count--;
		}

		dbb++;
	}

	proc->update_count = count;

	if (rc) return 0;
	return 1;
}

void backup_clear (PROC_t *proc)
{
	int i;
	DBB_t  *dbb;

	dbb	 = proc->dbb;
	for (i = 0; i < proc->update_count; i++) {

		if (access (dbb->bak_file, F_OK) != 0) {
			__AGENT_DBGP (__INFO, "No backup File Exists (%s)\n", dbb->bak_file);
		} else {
			unlink (dbb->bak_file);
		}

		dbb++;
	}

	return ;
}

int createTable_category (char *db, int index)
{
	int rc = 0;

	switch (index) {
    case TABLE_WEB_RULESET:
    case TABLE_WEBUSER_RULESET:
			rc = create_table_webcgi (db);
			break;
    case TABLE_INTERNAL_ANOMALY_RULESET:
			rc = create_table_anomaly (db);
			break;
    case TABLE_PB_RULESET:
    case TABLE_USER_RULESET:
			rc = create_table_patternblock (db);
			break;
    case TABLE_10G_HM_TABLE:
			rc = create_table_hm (db);
			break;
    case TABLE_RULESET_EIP3:
			rc = create_table_eip3 (db);
			break;
    case TABLE_SNORT_RULESET:
			rc = create_table_snort (db);
			break;
    case TABLE_DNS_RULESET:
			rc = create_table_dnsurl (db);
			break;
    case TABLE_ASIG_RULESET:
			rc = create_table_autosig (db);
			InitializeAsigDB (db);
			break;
    case TABLE_INTERNAL_RULESET:
			rc = create_table_internal (db);
			break;
    case TABLE_IPPOOL_RULESET:
			rc = create_table_ippool (db);
			break;
    case TABLE_VIPS_RULESET:
			rc = create_table_vips3 (db);
			break;
    case TABLE_VIPS_ANOMALY_RULESET:
			rc = create_table_vips_anomaly (db);
			break;
    case TABLE_NQOS:
			rc = create_table_nqos (db);
			break;
    case TABLE_APPS_GRP:
			rc = create_table_nqos_grp (db);
			break;
    case TABLE_YARA_RULESET:
			rc = create_table_yara (db);
			break;
    case TABLE_AR_CONTROL_RULE:
			rc = create_table_ar_policy (db);
			break;
    case TABLE_IPR_CONTROL_RULE:
			rc = create_table_ipr_policy (db);
			break;
		default:
			//error
			break;
	}

	return rc;
}

int create_table_webcgi (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE backup( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE defaultTB( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_anomaly (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			"category      INT,"
			"code          INT,"
			"name          TEXT,"
			"bound_idx     INT,"
			"protocol      INT,"
			"port          INT,"
			"pps_flag      INT,"
			"nlimit        INT,"
			"nlimit_max    INT,"
			"nlimit_avg    INT,"
			"nlimit_min    INT,"
			"risk          INT,"
			"alarm         INT,"
			"detecting     INT,"
			"raw           INT,"
			"by_study      INT,"
			"detection_flag      INT"
			",ndelete       INT"
			");"
			"CREATE TABLE defaultTB( "
			"category      INT,"
			"code          INT,"
			"name          TEXT,"
			"bound_idx     INT,"
			"protocol      INT,"
			"port          INT,"
			"pps_flag      INT,"
			"nlimit        INT,"
			"nlimit_max    INT,"
			"nlimit_avg    INT,"
			"nlimit_min    INT,"
			"risk          INT,"
			"alarm         INT,"
			"detecting     INT,"
			"raw           INT,"
			"by_study      INT,"
			" detection_flag INT"
			",ndelete       INT"
			");"
			"CREATE UNIQUE INDEX _master_key ON master (category,code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (category,code);");

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_patternblock (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE backup( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE defaultTB( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_hm (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",method        TEXT"
			",pattern       TEXT"
			",mask					TEXT"
			",name          TEXT"
			");"
			"CREATE TABLE backup( "
			" code          INT"
			",filter        INT"
			",nlimit         INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete        INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",method        TEXT"
			",pattern       TEXT"
			",mask					TEXT"
			",name          TEXT"
			");"
			"CREATE TABLE defaultTB( "
			" code          INT"
			",filter        INT"
			",nlimit         INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete        INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",method        TEXT"
			",pattern       TEXT"
			",mask					TEXT"
			",name          TEXT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_eip3 (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" category      INT"
			",code          INT"
			",flag          TEXT"
			",proto					INT"
			",s_port				INT"
			",d_port				INT"
			",s_network     TEXT"
			",s_netmask     TEXT"
			",s_broadcast   TEXT"
			",d_network     TEXT"
			",d_netmask     TEXT"
			",d_broadcast   TEXT"
			",v_id					INT"
			");"
			"CREATE TABLE ddx_eip_table( "
			" category      INT"
			",code          INT"
			",flag          TEXT"
			",proto					INT"
			",s_port				INT"
			",d_port				INT"
			",s_network     TEXT"
			",s_netmask     TEXT"
			",s_broadcast   TEXT"
			",d_network     TEXT"
			",d_netmask     TEXT"
			",d_broadcast   TEXT"
			",o_id					INT"
			");"
			"CREATE UNIQUE INDEX _ddx_eip_table_key ON ddx_eip_table (o_id, category, code, "
			"flag, s_network, s_netmask, d_network, d_netmask);"
			"CREATE INDEX _ddx_eip_table_idx ON ddx_eip_table (code, category, flag, o_id);"
			"CREATE UNIQUE INDEX _master_key ON master (v_id, category, code, "
			"flag, s_network, s_netmask, d_network, d_netmask);"
			"CREATE INDEX _master_idx ON master (code, category, flag, v_id);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_snort (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",ncscrule       TEXT"
			");"
			"CREATE TABLE backup( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",ncscrule       TEXT"
			");"
			"CREATE TABLE defaultTB( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",nupdate       INT"
			",author         INT"
			",method        TEXT"
			",nocase        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",name          TEXT"
			",offset_flag   INT"
			",reqrsp        INT"
			",offset       INT"
			",protocol      INT"
			",sport         INT"
			",ncscrule       TEXT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt        INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_dnsurl (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",pkt_len1      INT"
			",pkt_len2      INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",nocase        TEXT"
			",method        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",offset        INT"
			",offset_flag   TEXT"
			",dns_querytype TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			");"
			"CREATE TABLE backup( "
			" code          INT"
			",filter        INT"
			",nlimit         INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete        INT"
			",stype         INT"
			",pkt_len1      INT"
			",pkt_len2      INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",nocase        TEXT"
			",method        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",offset        INT"
			",offset_flag   TEXT"
			",dns_querytype TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			");"
			"CREATE TABLE defaultTB( "
			" code          INT"
			",filter        INT"
			",nlimit        INT"
			",expire        INT"
			",risk          INT"
			",alarm         INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",raw           INT"
			",ipcnt         INT"
			",ndelete       INT"
			",stype         INT"
			",pkt_len1      INT"
			",pkt_len2      INT"
			",nupdate        INT"
			",reqrsp        TEXT"
			",nocase        TEXT"
			",method        TEXT"
			",abstract1     TEXT"
			",abstract2     TEXT"
			",offset        INT"
			",offset_flag   TEXT"
			",dns_querytype TEXT"
			",pattern       TEXT"
			",name          TEXT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (code);"
			"CREATE INDEX _pattern_key ON master (pattern, ndelete);"
			"CREATE INDEX _name_key ON master (name, ndelete);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_internal (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			"category      INT,"
			"code          INT,"
			"name          TEXT,"
			"filter        INT,"
			"nlimit        INT,"
			"expire        INT,"
			"risk          INT,"
			"alarm         INT,"
			"detecting     INT,"
			"blocking      INT,"
			"blackhole     INT,"
			"raw           INT,"
			"ipcnt         INT,"
			"overflow      INT,"
			"method        INT,"
			"abstract1     INT,"
			"abstract2     INT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE defaultTB( "
			"category      INT,"
			"code          INT,"
			"name          TEXT,"
			"filter        INT,"
			"nlimit        INT,"
			"expire        INT,"
			"risk          INT,"
			"alarm         INT,"
			"detecting     INT,"
			"blocking      INT,"
			"blackhole     INT,"
			"raw           INT,"
			"ipcnt         INT,"
			"overflow      INT,"
			"method        INT,"
			"abstract1     INT,"
			"abstract2     INT"
			",by_study      INT"
			",v6abstract1    INT "
			",v6abstract2    INT"
			",license_flag   INT"
			");"
			"CREATE TABLE erule_cnt( "
			" category      INT"
			",code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (category, code);"
			"CREATE UNIQUE INDEX _master_key ON master (category,code);"
			"CREATE UNIQUE INDEX _defaultTB_key ON defaultTB (category,code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_ippool (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf(sql, sizeof(sql),
			"PRAGMA empty_result_callbacks = 1;"
			"CREATE TABLE group_table( " // 그룹정보 테이블
			"g_id   INTEGER"  // 그룹번호
			",g_name TEXT"   // 그룹명
			");"
			"CREATE TABLE object_table( " // 객체정보 테이블
			"o_id   INTEGER"  // 객체번호
			",o_name TEXT"   // 객체명
			",g_id   INTEGER"  // 그룹번호
			",use    INTEGER"   // 사용여부
			",dynamic INTEGER"   // dynamin enable/disable 사용여부
			",v_id INTEGER"			//정책 템플릿 연결을 위한 정책 템플릿 ID
			");"
			"CREATE TABLE pool_table( " // 그룹객체 관계테이블
			"o_id    INTEGER"   // 객체번호
			",network INTEGER"   // 네트워크
			",netmask INTEGER"   // 마스크
			",vlan    INTEGER"   // VLAN정보 (사용X)
			",etc     TEXT"      // 기타정보
			");"
			" CREATE UNIQUE INDEX _group_table_key  ON group_table  (g_id);"
			" CREATE UNIQUE INDEX _object_table_key ON object_table (o_id);"
			" CREATE INDEX _pool_table ON pool_table (o_id);"
			" CREATE TABLE sss_rule ( "
			" code         INT"
			",protocol     INT"
			",sport        TEXT"
			",ndelete      INT"
			",detecting    INT"
			",blocking     INT"
			",alarm        INT"
			",risk         INT"
			",raw          INT"
			",ipcnt        INT"
			",session_flow INT"
			",out_session  INT"
			",learn_mode   INT" // null or 1이상 이면 학습 on
			",eport_enable INT"
			",eports		   TEXT"
			",ns_rlimit    INT"
			",nt_cps       INT"
			",timeslot     INT"
			",nupdate      INT"
			",author       INT"
			",name         TEXT"
			",by_study		 INT"
			",abuser_enable INT NOT NULL DEFAULT '0'"
			",abuser_time   INT NOT NULL DEFAULT '5'"
			",abuser_rt     INT NOT NULL DEFAULT '0'"
			",sc_cps       INT"
			",tmp_timeslot INT NOT NULL DEFAULT '10' "
			",pkt_cnt_check_enable INT NOT NULL DEFAULT '0'"
			",pkt_cnt_limit INT NOT NULL DEFAULT '10'"
			");"
			"CREATE TABLE sss_erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _sss_erule_cnt ON sss_erule_cnt (code);"
			"CREATE UNIQUE INDEX _sss_rule_key ON sss_rule (code);"
			" CREATE TABLE udp_ctl_rule ( "
			" code         INT"
			",protocol     INT"
			",sport        TEXT"
			",ndelete      INT"
			",detecting    INT"
			",blocking     INT"
			",alarm        INT"
			",risk         INT"
			",raw          INT"
			",ipcnt        INT"
			",by_study    				INT"
			",learn_mode          INT"
			",init_drop_time      INT"
			",auth_keep_time      INT" 
			",auth_ip_qos_limit   INT"  
			",no_auth_rate_limit  INT"
			",name         				TEXT"
			");"
			"CREATE TABLE udp_ctl_erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _udp_ctl_erule_cnt ON udp_ctl_erule_cnt (code);"
			"CREATE UNIQUE INDEX _udp_ctl_rule_key ON udp_ctl_rule (code);"
			"CREATE TABLE ipv6_pool_table( " // 그룹객체 관계테이블
			"o_id    INTEGER"   // 객체번호
			",network TEXT"   // 네트워크  to text
			",netmask INTEGER"   // 마스크
			",vlan    INTEGER"   // VLAN정보 (사용X)
			",etc     TEXT"      // 기타정보
			");"
			" CREATE INDEX _v6_pool_table ON ipv6_pool_table (o_id);"
			" CREATE TABLE ipv6_sss_rule ( "
			" code         INT"
			",protocol     INT"
			",sport        TEXT"
			",ndelete      INT"
			",detecting    INT"
			",blocking     INT"
			",alarm        INT"
			",risk         INT"
			",raw          INT"
			",ipcnt        INT"
			",session_flow INT"
			",out_session  INT"
			",learn_mode   INT" // null or 1이상 이면 학습 on
			",eport_enable INT"
			",eports		   TEXT"
			",ns_rlimit    INT"
			",nt_cps       INT"
			",timeslot     INT"
			",nupdate      INT"
			",author       INT"
			",name         TEXT"
			",by_study		 INT"
			",sc_cps       INT"
			",tmp_timeslot INT NOT NULL DEFAULT '10' "
			",pkt_cnt_check_enable INT NOT NULL DEFAULT '0'"
			",pkt_cnt_limit INT NOT NULL DEFAULT '10'"
			");"
			"CREATE TABLE ipv6_sss_erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _v6_sss_erule_cnt ON ipv6_sss_erule_cnt (code);"
			"CREATE UNIQUE INDEX _v6_sss_rule_key ON ipv6_sss_rule (code);"
			"CREATE TABLE ipv6_dns_ctl_erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _v6_dns_ctl_erule_cnt ON ipv6_dns_ctl_erule_cnt (code);"
			"CREATE TABLE noc_manage_config( "
			"o_id INTEGER PRIMARY KEY"  // 객체번호
			", g_id INTEGER"						// 그룹 번호
			", ips INTEGER"   // ips 기능 설정 정보
			", dns INTEGER"   // dns 기능 설정 정보
			", voip INTEGER"   // voip 기능 설정 정보
			", https INTEGER"   // https 기능 설정 정보
			", snort INTEGER"   // snort 기능 설정 정보
			", qos INTEGER"   // qos 기능 설정 정보
			", ddx INTEGER"   // ddx 기능 설정 정보
			", dhcp INTEGER"   // dhcp 기능 설정 정보
			", sc3r INTEGER"   // session control + 3R 기능 설정 정보
			", log INTEGER"   // log 기능 설정 정보
			", reputation INTEGER"
			");"	
			"CREATE TABLE rule_template_table( " // 정책 템플릿 관리 테이블
			"v_id INTEGER PRIMARY KEY"  // 정책 템플릿 ID
			", v_name TEXT"						// 정책 템플릿 이름
			");"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_vips3 (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" category      INT"
			",code          INT"
			",filter        INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",alarm         INT"
			",by_study      INT"
			",v_id          INT"
			",nlimit				INT"
			",expire				INT"
			",ipcnt					INT"
			",raw						INT"
			");"
			"CREATE TABLE backup( "
			" category      INT"
			",code          INT"
			",filter        INT"
			",detecting     INT"
			",blocking      INT"
			",blackhole     INT"
			",alarm         INT"
			",by_study      INT"
			",v_id          INT"
			",nlimit				INT"
			",expire				INT"
			",ipcnt					INT"
			",raw						INT"
			");"
			"CREATE TABLE erule_cnt( "
			" category			INT"
			",v_id          INT"
			",code          INT"
			",eipcnt 				INT"
			");"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (v_id, code, category);"
			"CREATE UNIQUE INDEX _master_key ON master (category,code,v_id);"
			"CREATE UNIQUE INDEX _backup_key ON backup (category,code,v_id);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_vips_anomaly (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"CREATE TABLE master( "
			" category      INT"
			",code          INT"
			",v_id     			INT"
			",detecting     INT"
			",nlimit        INT"
			",nlimit_max    INT"
			",nlimit_avg    INT"
			",nlimit_min    INT"
			",alarm         INT"
			",by_study      INT"
			",detection_flag INT"
			",raw						INT"
			");"
			"CREATE TABLE backup( "
			" category      INT"
			",code          INT"
			",v_id     			INT"
			",detecting     INT"
			",nlimit        INT"
			",nlimit_max    INT"
			",nlimit_avg    INT"
			",nlimit_min    INT"
			",alarm        INT"
			",by_study     INT"
			",detection_flag    INT"
			",raw						INT"
			");"
			"CREATE UNIQUE INDEX _master_key ON master (category,code,v_id);"
			"CREATE UNIQUE INDEX _backup_key ON backup (category,code,v_id);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_nqos (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"PRAGMA empty_result_callbacks = 1;"
			"CREATE TABLE master ( " 
			" priority 		INT" 				// 1, reserv 
			",nupdate		  INT"				// 2, 등록시간
			",interface		INT"				// 3, 0/1/2/3, 4->any
			",protocol		INT"				// 4, 255->any
			",o_id_s			INT"				// 5, 
			",o_id_d			INT"				// 6, 
			",sip					INT"				// 7, source ip 기준
			",smask				INT"				// 8, 
			",dip					INT"				// 9, source ip 기준
			",dmask				INT"				// 10, 
			",sport				TEXT"				// 11, source port 기준
			",dport				TEXT"				// 12, source port 기준
			",option			TEXT"				// 13, tcp-syn/....
			",nlimit			INT"				// 14, 0~0xffffffff
			",type				INT"				// 15, pps-1
			",desc				TEXT"				// 16, 
			",etc					TEXT"				// 17

			",code				INT"				// 18
			",use					INT"				// 19
			",alarm				INT"				// 20
			",ndelete			INT"				// 21, reserv
			",detecting		INT"				// 22
			",blocking 		INT"				// 23
			",ipv			 		INT"				// 24
			");"
			" CREATE UNIQUE INDEX _master_table_key  ON master (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_nqos_grp (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf (sql, sizeof(sql),
			"PRAGMA empty_result_callbacks = 1;"
			"CREATE TABLE group_table( "
			"g_id   INTEGER"
			",g_name TEXT"
			");"
			"CREATE TABLE object_table( "
			" o_id   INTEGER"
			",o_name TEXT"
			",g_id   INTEGER"
			",use    INTEGER"
			");"
			"CREATE TABLE pool_table( "
			" o_id    INTEGER"
			",network INTEGER"
			",netmask INTEGER"
			",vlan    INTEGER"
			",etc     TEXT"
			");"
			"CREATE TABLE pool_table_ipv6( "
			" o_id    INTEGER"
			",network TEXT"
			",netmask INTEGER"
			",vlan    INTEGER"
			",etc     TEXT"
			");"
			" CREATE INDEX _pool_table_ipv6 ON pool_table_ipv6 (o_id);"
			" CREATE UNIQUE INDEX _group_table_key  ON group_table  (g_id);"
			" CREATE UNIQUE INDEX _object_table_key ON object_table (o_id);"
			" CREATE INDEX _pool_table ON pool_table (o_id);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_yara (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	sprintf (sql,
			"CREATE TABLE master( "
			" code          INT"
			",detecting     INT"
			",ndelete       INT"
			",name          TEXT"
			",yaraname      TEXT PRIMARY KEY"
			",yararule       TEXT"
			");"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_ar_policy (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf(sql, sizeof(sql), 
			"CREATE TABLE ar_control_rule("
			"code INT PRIMARY KEY NOT NULL"
			", detecting INT"
			", control   INT"
			", ipcnt     INT"
			");"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

int create_table_ipr_policy (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf(sql, sizeof(sql), 
			"CREATE TABLE reputation_policy "
			"( "
			"	policy_id INTEGER PRIMARY KEY, "
			"	type_id INTEGER, "
			"	cate_id INTEGER, "
			"	rule_id INTEGER, "
			"	pool_id INTEGER, "
			"	schedule_id INTEGER, "
			"	desc STRING, "
			"	score INTEGER DEFAULT -1, "
			"	policy INTEGER, "
			" filter INTEGER DEFAULT 0, "
			"	priority INTEGER, "
			"	risk INTEGER, "
			"	alert INTEGER, "
			"	raw INTEGER, "
			"	note STRING, "
			"	UNIQUE (type_id, cate_id, rule_id, pool_id) "
			"); "
			"CREATE INDEX reputation_policy_index ON reputation_policy (type_id, cate_id, rule_id); "
			"CREATE TABLE reputation_network "
			"( "
			"	policy_id INTEGER REFERENCES reputation_policy(policy_id) ON DELETE CASCADE, "
			"	score INTEGER DEFAULT -1, "
			"	ver INTEGER, "
			"	naddr STRING, "
			"	cidr INTEGER "
			"); "
			"CREATE INDEX reputation_network_index ON reputation_network(policy_id); "
			"CREATE TABLE reputation_url "
			"( "
			"	policy_id INTEGER REFERENCES reputation_policy(policy_id) ON DELETE CASCADE, "
			"	score INTEGER DEFAULT -1, "
			"	domain STRING, "
			"	uri STRING "
			"); "
			"CREATE INDEX reputation_url_index ON reputation_url(policy_id); "
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

void InitializeAsigDB (char *db)
{
	char *pquery;
	int  rc, i = 0;

	while (asig_default[i].code != 0) {
		pquery = sqlite3_mprintf (	
				" INSERT OR REPLACE INTO master ( "
				"        code,          "
				"        stype,         "
				"        protocol,      "
				"        detecting,     "
				"        blocking,      "
				"        alarm,         "
				"        risk,          "
				"        raw,           "
				"        ipcnt,         "
				"        ekey_cnt,      "
				"        min_pps,       "
				"        nupdate,       "
				"        author,        "
				"        name           "
				" ) VALUES (            "
				"  '%u', '%u', '%u', '%u', '%u' "
				" ,'%u', '%u', '%u', '%u', '%u' "
				" ,'%u', '%u', '%u', %Q); "
				,asig_default[i].code
				,asig_default[i].stype
				,asig_default[i].protocol
				,asig_default[i].detecting
				,asig_default[i].blocking
				,asig_default[i].alarm
				,asig_default[i].risk
				,asig_default[i].raw
				, 0
				, 0
				,asig_default[i].min_pps
				,asig_default[i].nupdate
				,RULE_AUTHOR_WINS
				,asig_default[i].name);

		i++;

		if (pquery) {
			rc = ExecDBQuery (db, pquery);
			sqlite3_free (pquery);
			pquery = NULL;
		}
	} //end while

	return ;
}

int create_table_autosig (char *db)
{
	int rc = SQLITE_OK;
	char sql[8192];

	snprintf(sql, sizeof(sql), 
			"CREATE TABLE master( "
			" code              INT"
			",stype             INT"
			",protocol          INT"
			",detecting         INT"
			",blocking          INT"
			",alarm             INT"
			",risk              INT"
			",raw               INT"
			",ipcnt             INT"
			",ekey_cnt          INT"
			",min_pps           INT"
			",nupdate           INT"
			",author            INT"
			",name              TEXT"
			");"
			"CREATE TABLE backup( "
			" code              INT"
			",stype             INT"
			",protocol          INT"
			",detecting         INT"
			",blocking          INT"
			",alarm             INT"
			",risk              INT"
			",raw               INT"
			",ipcnt             INT"
			",ekey_cnt          INT"
			",min_pps           INT"
			",nupdate           INT"
			",author            INT"
			",name              TEXT"
			");"
			"CREATE TABLE ekey_table( "
			" code              INT"
			",key         			INT"
			");"
			"CREATE TABLE erule_cnt( "
			" code          INT"
			",eipcnt 				INT"
			");"
			"CREATE TABLE e_key_cnt( "
			" code          INT"
			",ekey_cnt			INT"
			");"
			"CREATE UNIQUE INDEX _master_key ON master (code);"
			"CREATE UNIQUE INDEX _backup_key ON backup (code);"
			"CREATE UNIQUE INDEX _erule_cnt ON erule_cnt (code);"
			"CREATE UNIQUE INDEX _ekey_cnt ON e_key_cnt (code);"
			"CREATE UNIQUE INDEX _ekey_table ON ekey_table (code, key);"
			);

	rc = ExecDBQuery (db, sql);
	if (rc != SQLITE_OK) {	
		__AGENT_DBGP (__FAIL, "DB Create Fail (%s)\n", db);
		return -1;
	}

	return 0;
}

/**
 * @brief sn_worker 메인 실행 함수
 *
 * @param data 
 *
 * @return 
 */
int sn_worker_manager (DATA_t *data)
{
	int	rc			 = STATUS_SUCCESS;
	char g_update_date[64] = {0,}; /** @var */

	__AGENT_DBGP(__INFO, "Worker Start code[%s]\n", data->type_code);

	if (!strlen(g_update_date))
		snprintf (g_update_date, sizeof(g_update_date), "%s", data->update_date);

	if(create_loadpattern_file (g_update_date) < 0) {
		__AGENT_DBGP(__FAIL,"Worker: Make LoadLock Fail\n");
		data->status_code = STAT_COMPILE_SET_ERR;
		rc = STATUS_FAIL;
		goto manager_end;	
	}

	if(data->type_code[TYPE_SIDE] == '1'){
		switch (data->type_code[TYPE_MAJOR]) {
			case '1':	/* config */
				break;
			case '2':	/* pattern */
				rc = setup_pattern (data);
				goto manager_end;	
				break;
			case '3':	/* sn_spu */
				rc = rmsa_patch_func (data);
				break;
			case '4':	/* rec_engine */
				rc = setup_rec_engine (data);
				goto manager_end;	
				break;
			default:
				data->status_code = STAT_NOT_SUPPORT_CODE_ERR;
				rc = RET_FAIL;
				break;
		}
	}
	else if(data->type_code[TYPE_SIDE] == '2'){
		switch (data->type_code[TYPE_MAJOR]) {
			case '2':
				rc = get_upload_file(data);
				break;
			default:
				data->status_code = STAT_NOT_SUPPORT_CODE_ERR;
				rc = RET_FAIL;
				break;
		}
	}
	else{
		data->status_code = STAT_NOT_SUPPORT_CODE_ERR;
		rc = RET_FAIL;
	}
  TESaaT();

manager_end:

	return rc;
}

char *board_calMulti(int *icons)
{
	if(icons[3] == E_ICON_MUL2){
		//if(strstr(totalBuf, ".") != NULL){
		//	char *ptr1, *ptr2, *noDotBuf = (char *)malloc(sizeof(char)*4*BOARD_COLUMN);
		//	memset(noDotBuf, 0, sizeof(noDotBuf));
		//	ptr1 = strchr(totalBuf, '.') - 1;//do not use "
		//	ptr2 = strchr(totalBuf, '.') + 1;
		//	
		//	#ifdef PRINT_BRD_CALMULTI
		//	printf("totalBuf:%s, ptr1:%s, ptr2:%s, noDotBuf:%s, line:%d in board_calMulti()\n", totalBuf, ptr1, ptr2, noDotBuf, __LINE__);
		//	#endif	
		//}
		if(strstr(totalBuf, "1") != NULL)
			strreplace(totalBuf, "1", "2");
		else if(strstr(totalBuf, "5") != NULL)
			strreplace(totalBuf, "5", "10");
		#ifdef PRINT_BRD_CALMULTI
		printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
		#endif
	}else if(icons[3] == E_ICON_MUL5){
		if(strstr(totalBuf, "1") != NULL)
			strreplace(totalBuf, "1", "5");
		else if(strstr(totalBuf, "5") != NULL)
			strreplace(totalBuf, "5", "25");
		#ifdef PRINT_BRD_CALMULTI
		printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
		#endif		
	}else if(icons[3] == E_ICON_MUL10){
		if(strstr(totalBuf, ".") != NULL){
			if((icons[0] == E_ICON_PIPC000) || (icons[0] == E_ICON_PIPC00) || (icons[0] == E_ICON_PIPC0)){
				//char *noDotBuf = (char *)malloc(sizeof(char)*4*BOARD_COLUMN);
				//memset(noDotBuf, 0, sizeof(noDotBuf));
				//ptr1 = strchr(totalBuf, '.') - 1;//do not use "
				totalBuf = strchr(totalBuf, '.') + 1;
				#ifdef PRINT_BRD_CALMULTI
				printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
				#endif
			}else{
				char *ptr;
				ptr = strchr(totalBuf, '.');//do not use "
				memmove(totalBuf);
				#ifdef PRINT_BRD_CALMULTI
				printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
				#endif
			}			
		}else{
			strncat(totalBuf, "0", 1);
			#ifdef PRINT_BRD_CALMULTI
			printf("totalBuf:%s, line:%d in board_calMulti()\n", totalBuf, __LINE__);
			#endif	
		}
	}
}

char *strreplace(char *s, const char *s1, const char *s2)
{
    char *p = strstr(s, s1);
    if (p != NULL) {
        size_t len1 = strlen(s1);
        size_t len2 = strlen(s2);
		#ifdef PRINT_STR_REPLACE
		printf("len1:%d, len2:%d in line:%d, function:%s\n", len1, len2,  __LINE__, __FUNCTION__);
		#endif
        if (len1 != len2){
            memmove(p + len2, p + len1, strlen(p + len1) + 1);
			#ifdef PRINT_STR_REPLACE
			printf("p+len2:%s, p+len1:%s, strlen(p+len1)+1:%d in line:%d, function:%s\n", p + len2, p + len1, strlen(p + len1) + 1,  __LINE__, __FUNCTION__);
			#endif
		}
        memcpy(p, s2, len2);
		#ifdef PRINT_STR_REPLACE
		printf("p:%s, s2:%s, len2:%d in line:%d, function:%s \n", p, s2, len2, __LINE__, __FUNCTION__);
		#endif
    }
    return s;
}
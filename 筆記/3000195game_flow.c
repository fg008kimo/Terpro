
下面初始值都沒改 都是東元給我的設定參數
algMammon.c:main()->test_main()
TestMain.c:test_main()->test_main_real()
->protocol_mammonKillFish()(真正運算遊戲的地方)->protocol_mammonKillFish_2()//BET_LINE_NUM=25(最多壓線數), TABLE_ID=1
	Alg2.c:alg_kill_fish()//STR_AlgKillFishResultTemp gResult in algMammon.h;
		->table_in(tableId, totalBet)//tableId=1, totalBet=0 這裡只是在累加betCoins
		->is_dajiang_mode() //now disable
		->board.c:board_gen()->utils.c:GetRand()->GetRand_2()->rng.c:RngGetRand()->RngGetTempRandom()(非__LINUX__模式)->srand(time(NULL));(非__LINUX__模式)
			*RngGetTempRandom()在非linux模式return (rand() % RAND_MAX) / RAND_MAX)
			*RngGetRand 在非linux模式return ((rand() % RAND_MAX) / RAND_MAX) * (maxValue - minValue)) + minValue //minValue = 0, maxValue = rollerIconNum[column]
			*utils.c:GetRand()//中的 rollerIconNum[column] = maxValue
		->board_calPrizeType()->board_calIconNum() //return E_PRIZETYPE_FREEGAME or E_PRIZETYPE_NORMAL
		->board_calMulti()
		if E_PRIZETYPE_FREEGAME //play free game
			->freeGame_calNum()
			->freeGame_gen()
			->board_calMulti()
		->get_totalWinMulti()
		->table_out()
		->cpy_result()
	param_handle()->freeGame_handle()
	dy_handle()
->protocol_mammonGetData()->Alg2.c:alg_get_alg_data() //get cal data
->param_print()->freeGame_print() //print free game result
->dy_print() //print dy result
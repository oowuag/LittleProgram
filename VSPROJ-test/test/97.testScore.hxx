#include <stdio.h>
#include <complex>
#include <ctime>
#include <vector>
#include <algorithm>
using namespace std;

#define SCORE_AREA_1 0
#define SCORE_AREA_2 400
#define SCORE_AREA_3 800
#define SCORE_AREA_4 1100
#define SCORE_AREA_5 1400
#define SCORE_AREA_6 1700
#define SCORE_AREA_7 2000
#define SCORE_AREA_8 3000


struct Player
{
    int id;
	int score;
	int matches;
	int win;
	int lose;
	int draw;
};

bool palying(Player& kingA, Player& kingB)
{
    double x = (double)std::rand() / RAND_MAX;

    Player* pWinner = NULL;
    Player* pLoser = NULL;

    if (x > 0.6)
    {
        pWinner = &kingA;
        pLoser = &kingB;
    }
    else if (x < 0.4)
    {
        pWinner = &kingB;
        pLoser = &kingA;
    }
    else
    {
        kingA.matches++;
        kingA.draw++;
        kingB.matches++;
        kingB.draw++;
        return true;
    }

    pWinner->matches++;
    pWinner->win++;
    pLoser->matches++;
    pLoser->lose++;

    int aveScore = (pWinner->score + pLoser->score) / 2;
    int diffScore = pWinner->score - pLoser->score;

    if (aveScore >= SCORE_AREA_1 && aveScore < SCORE_AREA_2)
    {
        if (diffScore > 150) {
            pWinner->score += 22;
            pLoser->score -= 10;
        }
        if (diffScore > 100) {
            pWinner->score += 26;
            pLoser->score -= 14;
        }
        else if (diffScore > 50) {
            pWinner->score += 30;
            pLoser->score -= 18;
        }
        else if (diffScore < -50) {
            pWinner->score += 32;
            pLoser->score -= 10;
        }
        else if (diffScore < -100) {
            pWinner->score += 34;
            pLoser->score -= 12;
        }
        else if (diffScore < -150) {
            pWinner->score += 37;
            pLoser->score -= 15;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 18;
        }
    }
    else if (aveScore >= SCORE_AREA_2 && aveScore < SCORE_AREA_3)
    {
        if (diffScore > 150) {
            pWinner->score += 22;
            pLoser->score -= 12;
        }
        if (diffScore > 100) {
            pWinner->score += 26;
            pLoser->score -= 16;
        }
        else if (diffScore > 50) {
            pWinner->score += 30;
            pLoser->score -= 20;
        }
        else if (diffScore < -50) {
            pWinner->score += 32;
            pLoser->score -= 12;
        }
        else if (diffScore < -100) {
            pWinner->score += 34;
            pLoser->score -= 14;
        }
        else if (diffScore < -150) {
            pWinner->score += 37;
            pLoser->score -= 17;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 20;
        }
    }
    else if (aveScore >= SCORE_AREA_3 && aveScore < SCORE_AREA_4)
    {
        if (diffScore > 150) {
            pWinner->score += 20;
            pLoser->score -= 18;
        }
        if (diffScore > 100) {
            pWinner->score += 24;
            pLoser->score -= 22;
        }
        else if (diffScore > 50) {
            pWinner->score += 28;
            pLoser->score -= 26;
        }
        else if (diffScore < -50) {
            pWinner->score += 30;
            pLoser->score -= 28;
        }
        else if (diffScore < -100) {
            pWinner->score += 32;
            pLoser->score -= 30;
        }
        else if (diffScore < -150) {
            pWinner->score += 35;
            pLoser->score -= 33;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 36;
        }
    }
    else if (aveScore >= SCORE_AREA_4 && aveScore < SCORE_AREA_5)
    {
        if (diffScore > 150) {
            pWinner->score += 20;
            pLoser->score -= 20;
        }
        if (diffScore > 100) {
            pWinner->score += 24;
            pLoser->score -= 24;
        }
        else if (diffScore > 50) {
            pWinner->score += 28;
            pLoser->score -= 28;
        }
        else if (diffScore < -50) {
            pWinner->score += 30;
            pLoser->score -= 30;
        }
        else if (diffScore < -100) {
            pWinner->score += 32;
            pLoser->score -= 32;
        }
        else if (diffScore < -150) {
            pWinner->score += 35;
            pLoser->score -= 35;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 38;
        }
    }
    else if (aveScore >= SCORE_AREA_6 && aveScore < SCORE_AREA_7)
    {
        if (diffScore > 150) {
            pWinner->score += 20;
            pLoser->score -= 20;
        }
        if (diffScore > 100) {
            pWinner->score += 24;
            pLoser->score -= 24;
        }
        else if (diffScore > 50) {
            pWinner->score += 28;
            pLoser->score -= 28;
        }
        else if (diffScore < -50) {
            pWinner->score += 30;
            pLoser->score -= 30;
        }
        else if (diffScore < -100) {
            pWinner->score += 32;
            pLoser->score -= 32;
        }
        else if (diffScore < -150) {
            pWinner->score += 35;
            pLoser->score -= 35;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 38;
        }
    }
    else if (aveScore >= SCORE_AREA_7 && aveScore < SCORE_AREA_8)
    {
        if (diffScore > 150) {
            pWinner->score += 20;
            pLoser->score -= 20;
        }
        if (diffScore > 100) {
            pWinner->score += 24;
            pLoser->score -= 24;
        }
        else if (diffScore > 50) {
            pWinner->score += 28;
            pLoser->score -= 28;
        }
        else if (diffScore < -50) {
            pWinner->score += 30;
            pLoser->score -= 30;
        }
        else if (diffScore < -100) {
            pWinner->score += 32;
            pLoser->score -= 32;
        }
        else if (diffScore < -150) {
            pWinner->score += 35;
            pLoser->score -= 35;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 38;
        }
    }
    else {
        if (diffScore > 150) {
            pWinner->score += 20;
            pLoser->score -= 20;
        }
        if (diffScore > 100) {
            pWinner->score += 24;
            pLoser->score -= 24;
        }
        else if (diffScore > 50) {
            pWinner->score += 28;
            pLoser->score -= 28;
        }
        else if (diffScore < -50) {
            pWinner->score += 30;
            pLoser->score -= 30;
        }
        else if (diffScore < -100) {
            pWinner->score += 32;
            pLoser->score -= 32;
        }
        else if (diffScore < -150) {
            pWinner->score += 35;
            pLoser->score -= 35;
        }
        else {
            pWinner->score += 38;
            pLoser->score -= 38;
        }
    }

    if (pLoser->score < 0) {
        pLoser->score = 0;
    }

    //printf("%8dvs%d %8d:%d\n", kingA.id, kingB.id, kingA.score, kingB.score);

    return true;
}

bool cmpfunction(Player i,Player j)
{
    return (i.score > j.score);
}

int main()
{
	const int PLAYER_N = 1000;
	const int DAY_N = 40;
	const int MATCH_PERDAY_N = 15; 
	
	Player player[PLAYER_N] = { 0 };
    for (int p = 0; p < PLAYER_N; p++)
    {
        player[p].id = p;
    }

	std::srand(std::time(0));

	for (int d = 0; d < DAY_N; d++)
    {
		for (int p = 0; p < PLAYER_N; p++)
        {
			for (int m = 0; m < MATCH_PERDAY_N; m++)
            {
                // select players
                std::vector<Player*> vecMatch;
                for (int pq = 0; pq < PLAYER_N; pq++)
                {
                    if ((pq != p) && abs(player[pq].score - player[p].score) <= 200)
                    {
                        vecMatch.push_back(&player[pq]);
                    }
                }

                int q = (double)std::rand() / RAND_MAX * vecMatch.size();
                if (q != p && q < vecMatch.size())
                {
                    palying(player[p], *vecMatch[q]);
                }
			}
		}
	}

    std::vector<Player> vecPlayers;;
    for (int p = 0; p < PLAYER_N; p++)
    {
        vecPlayers.push_back(player[p]);
    }

    std::sort(vecPlayers.begin(), vecPlayers.end(), cmpfunction);


    printf("###################################\n");

    FILE *pFile = fopen("D:\\testScore.csv", "wb");

    char *buff = new char[1024];

    std::vector<Player>::iterator it;
    for (it = vecPlayers.begin(); it != vecPlayers.end(); it++)
    {
        sprintf(buff, "%4d, %4d, %4d\n", it->id, it->score,it->matches);
        fwrite(buff, 1, strlen(buff), pFile);
    }

    delete[] buff;
    fclose(pFile);
   
	return 0;
}
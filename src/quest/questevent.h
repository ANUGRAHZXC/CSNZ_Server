#pragma once

#include "room/gamematch.h"
#include "manager/packetmanager.h"
#include "common/utils.h"

class CQuestEvent;
class CQuestEventTask;
class CQuestEventBaseCondition
{
public:
	CQuestEventBaseCondition(CQuestEventTask* task, int eventType, int id, int goal);

	bool Event_Internal(IUser* user);

	int GetID();
	int GetGoalPoints();
	int GetEventType();

protected:
	CQuestEventTask* m_pTask;
	int m_nID;
	int m_nEventType;
	int m_nGoalPoints;
};

class CQuestEventTask
{
public:
	CQuestEventTask(CQuestEvent* quest, int id, int goal);
	~CQuestEventTask();

	void IncrementCount(IUser* user, int count = 0, bool setForce = false);
	void Done(IUser* user, UserQuestTaskProgress& progress);
	void SetNotice(int goal, std::string userMsg);
	void SetRewardID(int rewardID);
	void AddCondition(CQuestEventBaseCondition* condition);
	void ApplyProgress(IUser* user, UserQuestTaskProgress& progress);

	void OnMinuteTick(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnKillEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, GameMatch_KillEvent& killEvent);

	void OnBombExplode(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnBombDefuse(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnHostageEscape(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnMonsterKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int monsterType);
	void OnMosquitoKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnKiteKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch);

	void OnLevelUpEvent(IUser* user, int level, int newLevel);
	void OnGameMatchLeave(IUser* user);
	void OnMatchEndEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int userTeam);
	void OnUserLogin(IUser* user);

	int GetID();
	int GetGoal();
	int GetNoticeGoal();
	std::string GetNoticeUserMsg();
	int GetRewardID();
	CQuestEvent* GetQuest();
	const std::vector<CQuestEventBaseCondition*>& GetConditions();
	bool IsFinished(IUser* user);

protected:
	CQuestEvent* m_pQuest;
	int m_nID;
	int m_nGoal;
	int m_nRewardID;
	int m_nNoticeGoal;
	std::string m_szNoticeUserMsg;
	std::vector<CQuestEventBaseCondition*> m_Conditions;
};

class CQuestEvent
{
public:
	CQuestEvent();
	~CQuestEvent();

	void SetID(int id);
	int GetID();
	void AddTask(CQuestEventTask* task);
	CQuestEventTask* GetTask(int id);
	std::vector<CQuestEventTask*>& GetTasks();

	void ApplyProgress(IUser* user, UserQuestProgress& progress);

	void OnMinuteTick(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnKillEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, GameMatch_KillEvent& killEvent);
	void OnBombExplode(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnBombDefuse(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnHostageEscape(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnMonsterKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int monsterType);
	void OnMosquitoKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch);
	void OnKiteKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch);

	void OnLevelUpEvent(IUser* user, int level, int newLevel);
	void OnGameMatchLeave(IUser* user);
	void OnMatchEndEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int userTeam);
	void OnUserLogin(IUser* user);

	void OnTaskDone(IUser* user, UserQuestTaskProgress& taskProgress, CQuestEventTask* task);
	bool IsAllTaskFinished(IUser* user);

private:
	int m_nID;
	std::vector<CQuestEventTask*> m_Tasks;
};

class CQuestEventBaseConditionGameMatch : public CQuestEventBaseCondition
{
public:
	CQuestEventBaseConditionGameMatch(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, const std::vector<int>& maps, int playerCount) : CQuestEventBaseCondition(task, eventType, id, goal)
	{
		m_GameModes = gameModes;
		m_Maps = maps;
		m_nPlayerCount = playerCount;
	}

	bool Event_Internal(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseCondition::Event_Internal(userStat->m_pUser))
			return false;

		if (m_GameModes.size() > 0 && std::find(m_GameModes.begin(), m_GameModes.end(), gameMatch->m_nGameMode) == m_GameModes.end())
			return false;

		if (m_Maps.size() > 0 && userStat->m_pUser->GetCurrentRoom() && std::find(m_Maps.begin(), m_Maps.end(), userStat->m_pUser->GetCurrentRoom()->GetSettings()->mapId) == m_Maps.end())
			return false;

		if (m_nPlayerCount > 0 && m_nPlayerCount > (int)gameMatch->m_UserStats.size())
			return false;

		return true;
	}

	void IncrementTempCount(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		UserQuestTaskProgress& tempProgress = userStat->GetTempQuestEventTaskProgress(m_pTask->GetQuest()->GetID(), m_pTask->GetID());
		tempProgress.unitsDone += m_nGoalPoints;
		if (m_pTask->GetNoticeGoal() > 0)
		{
			if (tempProgress.unitsDone % m_pTask->GetNoticeGoal() == 0)
			{
				g_PacketManager.SendUMsgNoticeMessageInChat(userStat->m_pUser->GetExtendedSocket(), va((char*)m_pTask->GetNoticeUserMsg().c_str(), tempProgress.unitsDone, m_pTask->GetGoal()));
			}
		}

		if (tempProgress.unitsDone >= m_pTask->GetGoal())
		{
			m_pTask->IncrementCount(userStat->m_pUser, m_pTask->GetGoal(), true);
		}
#ifdef _DEBUG
		Logger().Debug("[User '%s'] CQuestEventBaseConditionGameMatch::IncrementTempCount: quest id: %d, task id: %d, done: %d, goal: %d\n", userStat->m_pUser->GetLogName(), m_pTask->GetQuest()->GetID(), m_pTask->GetID(), tempProgress.unitsDone, m_pTask->GetGoal());
#endif
	}

private:
	std::vector<int> m_GameModes;
	std::vector<int> m_Maps;
	int m_nPlayerCount;
};

class CQuestEventConditionLevelUp : public CQuestEventBaseCondition
{
public:
	CQuestEventConditionLevelUp(CQuestEventTask* task, int eventType, int id, int goal) : CQuestEventBaseCondition(task, eventType, id, goal)
	{
	}

	void OnLevelUpEvent(IUser* user, int level, int newLevel)
	{
		if (!CQuestEventBaseCondition::Event_Internal(user))
			return;

		m_pTask->IncrementCount(user, newLevel - level);
	}
};

class CQuestEventConditionKill : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionKill(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
		m_nKillerTeam = 0;
		m_nVictimKillType = 0;
		m_nVictimTeam = 0;
		m_nContinuous = 0;
		m_nGunID = 0;
		m_nBot = 0;
		m_bHuman = 0;
		m_bCheckForGun = 0;
	}

	// TODO: check for position
	void OnKillEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, GameMatch_KillEvent killEvent)
	{
		if (!CQuestEventConditionKill::Event_Internal(userStat, gameMatch))
			return;

		if (m_nKillerTeam > 0 && m_nKillerTeam != killEvent.killerTeam)
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}

		if (m_nVictimKillType >= 0 && m_nVictimKillType != killEvent.victimKillType)
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}

		if (m_nVictimTeam > 0 && m_nVictimTeam != killEvent.victimTeam)
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}
		else if (m_nVictimTeam == 0 && killEvent.killerTeam == killEvent.victimTeam) // check if you killed a person from opposite team
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}

		if (m_nGunID >= 0 && m_nGunID != killEvent.gunID)
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}
		else if (m_bCheckForGun && m_nGunID == killEvent.gunID && !userStat->GetItem(killEvent.gunID))
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}

		if (m_nBot >= 0 && m_nBot ? killEvent.victimUserID != 0 : killEvent.victimUserID == 0)
		{
			ResetContinuousCounter(userStat, gameMatch);
			return;
		}

		UserQuestTaskProgress& tempProgress = userStat->GetTempQuestEventTaskProgress(m_pTask->GetQuest()->GetID(), m_pTask->GetID());
		if (m_nContinuous <= 0 || m_nContinuous <= tempProgress.killEventProgress.continuousKillsCounter)
		{
			IncrementTempCount(userStat, gameMatch);
			if (tempProgress.killEventProgress.continuousKillsCounter)
				ResetContinuousCounter(userStat, gameMatch);
		}
		else if (m_nContinuous > tempProgress.killEventProgress.continuousKillsCounter)
		{
			tempProgress.killEventProgress.continuousKillsCounter++;
		}
	}

	void SetCondition(int killerTeam, int victimKillType, int victimTeam, int continuous, int gunID, bool checkForGun, int bot)
	{
		m_nKillerTeam = killerTeam;
		m_nVictimKillType = victimKillType;
		m_nVictimTeam = victimTeam;
		m_nContinuous = continuous;
		m_nGunID = gunID;
		m_bCheckForGun = checkForGun;
		m_nBot = bot;
	}

	void ResetContinuousCounter(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (m_nContinuous > 0)
		{
			UserQuestTaskProgress& tempProgress = userStat->GetTempQuestEventTaskProgress(m_pTask->GetQuest()->GetID(), m_pTask->GetID());
			tempProgress.killEventProgress.continuousKillsCounter = 0;
		}
	}

private:
	int m_nKillerTeam;
	int m_nVictimKillType;
	int m_nVictimTeam;
	int m_nContinuous;
	int m_nGunID;
	bool m_bCheckForGun;
	int m_nBot;
	bool m_bHuman;
};

class CQuestEventConditionBombExplode : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionBombExplode(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnBombExplode(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionBombDefuse : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionBombDefuse(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnBombDefuse(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionVipKill : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionVipKill(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnVipKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionWin : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionWin(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnMatchEndEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int userTeam)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		if (userTeam == Terrorist && gameMatch->m_nTerWinCount < gameMatch->m_nCtWinCount)
			return;
		else if (userTeam == CounterTerrorist && gameMatch->m_nTerWinCount > gameMatch->m_nCtWinCount)
			return;

		m_pTask->IncrementCount(userStat->m_pUser);
	}
};

class CQuestEventConditionHostageEscape : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionHostageEscape(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnHostageEscape(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionTimeMatch : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionTimeMatch(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnMinuteTick(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionLogin : public CQuestEventBaseCondition
{
public:
	CQuestEventConditionLogin(CQuestEventTask* task, int eventType, int id, int goal) : CQuestEventBaseCondition(task, eventType, id, goal)
	{
	}

	void OnUserLogin(IUser* user)
	{
		if (!CQuestEventBaseCondition::Event_Internal(user))
			return;

		m_pTask->IncrementCount(user);
	}
};

class CQuestEventConditionKillMonster : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionKillMonster(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
		m_nMonsterType = 0;
	}

	void SetMonsterType(int monsterType)
	{
		m_nMonsterType = monsterType;
	}

	void OnMonsterKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int monsterType)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		if (monsterType == m_nMonsterType)
			IncrementTempCount(userStat, gameMatch);
	}

private:
	int m_nMonsterType;
};

class CQuestEventConditionKillMosquito : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionKillMosquito(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnMosquitoKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

class CQuestEventConditionKillKite : public CQuestEventBaseConditionGameMatch
{
public:
	CQuestEventConditionKillKite(CQuestEventTask* task, int eventType, int id, int goal, std::vector<int> gameModes, std::vector<int> maps, int playerCount) : CQuestEventBaseConditionGameMatch(task, eventType, id, goal, gameModes, maps, playerCount)
	{
	}

	void OnKiteKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
	{
		if (!CQuestEventBaseConditionGameMatch::Event_Internal(userStat, gameMatch))
			return;

		IncrementTempCount(userStat, gameMatch);
	}
};

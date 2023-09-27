#pragma once

#include "UserManager.h"
#include "Manager.h"
#include "ILuckyItemManager.h"

class CLuckyItemManager : public CBaseManager<ILuckyItemManager>
{
public:
	CLuckyItemManager();
	~CLuckyItemManager();

	virtual bool Init();
	virtual void Shutdown();

	void LoadLuckyItems();
	int OpenItemBox(CUser* user, int itemBox, int itemBoxOpenCount);
	std::vector<ItemBox*>& GetItemBoxes();
	std::vector<ItemBoxItem>& GetItems();
	ItemBox* GetItemBoxByItemId(int itemId);

private:
	bool KVToJson(); // TODO: delete sometime

	std::vector<ItemBoxItem> m_Items;
	std::vector<ItemBox*> m_ItemBoxes;
};

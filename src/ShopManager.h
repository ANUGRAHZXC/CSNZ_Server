#pragma once

#include "ItemManager.h"
#include "IShopManager.h"

class CShopManager : public CBaseManager, public IShopManager
{
public:
	CShopManager();
	~CShopManager();

	virtual bool Init();
	virtual void Shutdown();

	bool LoadProducts();

	void OnShopPacket(CReceivePacket* msg, CExtendedSocket* socket);
	void GetProductBySubId(int productId, Product& product, SubProduct& subProduct);
	bool BuyProduct(CUser* user, int productTypeId, int productId);
	
	const std::vector<Product>& GetProducts();
	const std::vector<std::vector<int>>& GetRecommendedProducts();
	const std::vector<int>& GetPopularProducts();

private:	
	bool KVToJson();

	std::vector<Product> m_Products;
	std::vector<std::vector<int>> m_RecommendedProducts;
	std::vector<int> m_PopularProducts;
};

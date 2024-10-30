#include "voxelmanager.h"
#include "packetmanager.h"
#include "serverconfig.h"

CVoxelManager g_VoxelManager;

CVoxelManager::CVoxelManager() : CBaseManager("VoxelManager")
{
}

CVoxelManager::~CVoxelManager()
{
}

bool CVoxelManager::OnPacket(CReceivePacket* msg, IExtendedSocket* socket)
{
	LOG_PACKET;

	int type = msg->ReadUInt8();
	switch (type)
	{
	case 4:
		g_PacketManager.SendVoxelUnk4(socket);
		break;
	case 8:
		g_PacketManager.SendVoxelUnk8(socket);
		break;
	case 9:
		g_PacketManager.SendVoxelUnk9(socket);
		break;
	case 10:
		g_PacketManager.SendVoxelUnk10(socket);
		break;
	case 38:
		g_PacketManager.SendVoxelUnk38(socket);
		break;
	case 46:
		g_PacketManager.SendVoxelUnk46(socket);
		break;
	case 47:
		g_PacketManager.SendVoxelUnk47(socket);
		break;
	case 58:
		g_PacketManager.SendVoxelUnk58(socket);
		break;
	default:
		Logger().Warn("Unknown voxel request %d\n", type);
		break;
	}

	return true;
}

static const int TIMEOUT = 3000;

std::string CVoxelManager::GetSlotDetails(const std::string& slotId)
{
	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, g_pServerConfig->voxelHTTPIP.c_str(), &servaddr.sin_addr) == 0)
	{
		Logger().Warn("CVoxelManager::GetSlotDetails: Error parsing host address.\n");
		return "";
	}
	servaddr.sin_port = htons(stoi(g_pServerConfig->voxelHTTPPort));

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&TIMEOUT), sizeof(TIMEOUT));

	if (sock < 0)
	{
		Logger().Warn("CVoxelManager::GetSlotDetails: Error creating socket.\n");
		return "";
	}

	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		closesocket(sock);
		Logger().Warn("CVoxelManager::GetSlotDetails: Could not connect.\n");
		return "";
	}

	std::stringstream ss;
	ss << "GET /v6/slots/detail/" << slotId.c_str() << " HTTP/1.1\r\n"
		<< "Connection: Keep-Alive\r\n"
		<< "User-Agent: cpprestsdk/2.10.2\r\n"
		<< "Host: " << g_pServerConfig->voxelHTTPIP << ":" << g_pServerConfig->voxelHTTPPort << "\r\n"
		<< "\r\n\r\n";
	std::string request = ss.str();

	if (send(sock, request.c_str(), request.length(), 0) != (int)request.length())
	{
		closesocket(sock);
		Logger().Warn("CVoxelManager::GetSlotDetails: Error sending request.\n");
		return "";
	}

	std::string response;
	char cur;
	bool found = false;
	while (recv(sock, &cur, 1, 0) > 0)
	{
		if (!found && cur == '{')
			found = true;
		
		if (found)
			response += cur;
	}

	closesocket(sock);
	return response;
}
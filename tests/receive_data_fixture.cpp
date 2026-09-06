// Transport-independent contract fixture. The runner inserts the actual
// CSteamPlayClient::ReceiveData definition from production, never a copy of it.
#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>
#include <stdexcept>

typedef unsigned int DWORD;
typedef DWORD DPID;
typedef DPID* LPDPID;
typedef DWORD* LPDWORD;
typedef void* LPVOID;
typedef int HRESULT;
const HRESULT DP_OK=0, DPERR_INVALIDPARAM=(int)0x80070057,
    DPERR_NOMESSAGES=(int)0x887700be, DPERR_BUFFERTOOSMALL=(int)0x8877001e;
const DWORD DPID_SYSMSG=0, DPRECEIVE_ALL=1, DPRECEIVE_TOPLAYER=2,
    DPRECEIVE_FROMPLAYER=4, DPRECEIVE_PEEK=8;
void check(bool condition,const char* text) { if(!condition) throw std::runtime_error(text); }
#define CHECK(condition) check((condition),#condition)

// Layout from Messages/Shared::SData: one-byte type, two DPID fields, payload.
// The real class header needs the private Steam SDK; this fixture only supplies
// the queue/message interface that ReceiveData consumes.
#pragma pack(push,1)
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif
namespace Messages { namespace Shared {
struct SData { unsigned char type; DPID from,to; char pData[0]; };
} }
#pragma pack(pop)
struct Message {
    std::vector<unsigned char> bytes;
    Message(size_t prefix,size_t size):bytes(prefix+size,0) {
        for(size_t i=0;i<size;++i) bytes[prefix+i]=(unsigned char)(i+1);
    }
    const void* GetData() const { if(bytes.empty()) return ""; return &bytes[0]; }
    size_t GetSize() const { return bytes.size(); }
};
class CSteamPlayClient {
public:
    struct Cached { DPID from,to; Message* pMsg; };
    typedef std::deque<Cached> TDataMessages;
    TDataMessages m_dataMessages;
    void add(DPID from,DPID to,Message& msg) {
        Cached item={from,to,&msg}; m_dataMessages.push_back(item);
    }
    HRESULT ReceiveData(LPDPID,LPDPID,DWORD,LPVOID,LPDWORD);
};

// PRODUCTION_RECEIVE_DATA

int main() {
    try {
        CHECK(sizeof(DWORD)==4 && sizeof(Messages::Shared::SData)==9);
        int cases=0;
        for(int system=0;system<2;++system) for(size_t length=0;length<=12;length+=3) {
            Message message(system?0:9,length);
            CSteamPlayClient client; client.add(system?0:123,456,message);
            DPID from=0,to=0; DWORD size=100;
            unsigned char buffer[32]; memset(buffer,0xa5,sizeof(buffer));
            CHECK(client.ReceiveData(&from,&to,0,0,&size)==DPERR_BUFFERTOOSMALL);
            CHECK(size==length && client.m_dataMessages.size()==1); ++cases;
            if(length) {
                size=(DWORD)length-1;
                CHECK(client.ReceiveData(&from,&to,0,buffer,&size)==DPERR_BUFFERTOOSMALL);
                CHECK(size==length && buffer[0]==0xa5 && client.m_dataMessages.size()==1); ++cases;
            }
            size=sizeof(buffer);
            CHECK(client.ReceiveData(&from,&to,DPRECEIVE_PEEK,buffer,&size)==DP_OK);
            CHECK(size==length && from==(system?0u:123u) && to==456);
            CHECK(client.m_dataMessages.size()==1 && buffer[length]==0xa5);
            for(size_t i=0;i<length;++i) CHECK(buffer[i]==i+1);
            ++cases;
            size=(DWORD)length;
            CHECK(client.ReceiveData(&from,&to,0,buffer,&size)==DP_OK);
            CHECK(size==length && client.m_dataMessages.empty()); ++cases;
            size=23;
            CHECK(client.ReceiveData(&from,&to,0,buffer,&size)==DPERR_NOMESSAGES);
            CHECK(size==23); ++cases;
        }
        Message first(9,3),second(9,6);
        CSteamPlayClient client; client.add(1,2,first); client.add(3,4,second);
        unsigned char buffer[32]; DWORD size=sizeof(buffer); DPID from=3,to=4;
        CHECK(client.ReceiveData(&from,&to,DPRECEIVE_FROMPLAYER|DPRECEIVE_TOPLAYER,buffer,&size)==DP_OK);
        CHECK(size==6 && client.m_dataMessages.size()==1 && client.m_dataMessages.front().from==1); ++cases;
        size=17; from=9;
        CHECK(client.ReceiveData(&from,&to,DPRECEIVE_FROMPLAYER,buffer,&size)==DPERR_NOMESSAGES);
        CHECK(size==17 && client.m_dataMessages.size()==1); ++cases;
        CHECK(client.ReceiveData(0,&to,0,buffer,&size)==DPERR_INVALIDPARAM);
        CHECK(client.ReceiveData(&from,0,0,buffer,&size)==DPERR_INVALIDPARAM);
        CHECK(client.ReceiveData(&from,&to,0,buffer,0)==DPERR_INVALIDPARAM);
        CHECK(client.m_dataMessages.size()==1); cases+=3;
        CHECK(client.ReceiveData(&from,&to,DPRECEIVE_ALL,buffer,&size)==DP_OK);
        CHECK(from==1 && to==2 && size==3 && client.m_dataMessages.empty()); ++cases;
        printf("PASS: %d ReceiveData contract cases\n",cases);
        return 0;
    } catch(const std::exception& e) {
        fprintf(stderr,"FAIL: %s\n",e.what()); return 1;
    }
}

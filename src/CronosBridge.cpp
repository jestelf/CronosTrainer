#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <sstream>
#include <unordered_map>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace rva {
 constexpr uint32_t Stamp=0xB3DA5796, Image=0x0BBB2000;
 constexpr uintptr_t GUObjectArray=0xAD91660, NamePool=0xACADEC0, ProcessEvent=0x14C2F90;
 constexpr uintptr_t GetPlayerCharacter=0x3927F30, GetCharacterASC=0x55C39B0, ToggleLooseTag=0x54FCAD0;
 constexpr uintptr_t EnemyStaticClass=0x53FAA10;

 constexpr uintptr_t GiveAllEquipment=0x55B6560, GiveAllItems=0x55B6760, GiveAllResources=0x55B6980, GiveAmmo=0x55B6AD0,
  KillAll=0x55B7780, UnlockAllUpgrades=0x55B7F80;
 constexpr uintptr_t TagUnlimitedAmmo=0xAF97FA0;
}
namespace mai { constexpr int Health=0xA4, Immortal=0xA8, OneHit=0xA9; }

namespace play { constexpr int HealthValue=0xD0, Data=0xA0, Protect=0x90, Cooldown=0x94; constexpr uintptr_t GetMaxHealth=0x560A730; }
static const uint8_t PEPrologue[19]={0x40,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x81,0xEC,0x00,0x01,0x00,0x00};

static uintptr_t gBase=0; static DWORD gGameThread=0; static bool gHooked=false;
using PEFn=void(__fastcall*)(void*,void*,void*);
static PEFn gOrigPE=nullptr; static uint8_t* gTramp=nullptr;

template<class T> static bool Rd(uintptr_t a,T& out){ if(a<0x10000) return false; __try{ out=*(T*)a; return true; }__except(EXCEPTION_EXECUTE_HANDLER){ return false; } }
template<class T> static T R(uintptr_t a){ T v{}; Rd(a,v); return v; }
template<class T> static bool Wr(uintptr_t a,T v){ if(a<0x10000) return false; __try{ *(T*)a=v; return true; }__except(EXCEPTION_EXECUTE_HANDLER){ return false; } }
static bool Readable(uintptr_t a){ uint8_t b; return Rd(a,b); }
static std::string Hex(uintptr_t v){ char b[32]; sprintf_s(b,"0x%llX",(unsigned long long)v); return b; }

static std::string NameFromIndex(uint32_t id){
 uint32_t block=id>>16, off=(id&0xFFFF)*2;
 if(block>=8192) return "";
 uintptr_t blk=R<uintptr_t>(gBase+rva::NamePool+0x10+8ull*block); if(!blk) return "";
 uintptr_t e=blk+off; uint16_t hdr=R<uint16_t>(e); int len=hdr>>6; bool wide=hdr&1;
 if(len<=0||len>1024) return "";
 std::string s; s.reserve(len);
 for(int i=0;i<len;i++){ if(wide){ wchar_t c=R<wchar_t>(e+2+2*i); s+=(c<128?(char)c:'?'); } else s+=R<char>(e+2+i); }
 return s;
}
static std::string FNameStr(uintptr_t fname){ uint32_t idx=R<uint32_t>(fname), num=R<uint32_t>(fname+4); std::string s=NameFromIndex(idx); if(num) s+="_"+std::to_string(num-1); return s; }

static uintptr_t ObjClass(uintptr_t o){ return R<uintptr_t>(o+0x10); }
static std::string ObjName(uintptr_t o){ return FNameStr(o+0x18); }
static int ObjCount(){ return R<int>(gBase+rva::GUObjectArray+0x10+0x14); }
static uintptr_t ObjAt(int i){
 uintptr_t chunks=R<uintptr_t>(gBase+rva::GUObjectArray+0x10); if(!chunks) return 0;
 uintptr_t chunk=R<uintptr_t>(chunks+8ull*(i/65536)); if(!chunk) return 0;
 return R<uintptr_t>(chunk+0x18ull*(i%65536));
}
static uintptr_t Super(uintptr_t st){ return R<uintptr_t>(st+0x40); }
static bool IsA(uintptr_t o,const char* cls){ int d=0; for(uintptr_t c=ObjClass(o);c && d<64;c=Super(c),d++) if(ObjName(c)==cls) return true; return false; }
static bool Derives(uintptr_t cls,uintptr_t base){ int d=0; for(uintptr_t c=cls;c && d<64;c=Super(c),d++) if(c==base) return true; return false; }
static bool IsTemplate(uintptr_t o){ return (R<uint32_t>(o+8)&0x30)!=0; }
static uintptr_t FindFirst(const char* cls){ int n=ObjCount(); for(int i=0;i<n;i++){ uintptr_t o=ObjAt(i); if(o && !IsTemplate(o) && IsA(o,cls)) return o; } return 0; }

struct Prop{ std::string name,type; int off=0,size=0; };

static Prop PropInfo(uintptr_t f){ Prop r; r.name=FNameStr(f+0x20); r.type=FNameStr(R<uintptr_t>(f+0x8)); r.off=R<int>(f+0x44); r.size=R<int>(f+0x34); return r; }
static std::vector<Prop> Props(uintptr_t cls){ std::vector<Prop> v; int d=0; for(uintptr_t c=cls;c && d<64;c=Super(c),d++){ int k=0; for(uintptr_t f=R<uintptr_t>(c+0x50);f && k<4096;f=R<uintptr_t>(f+0x18),k++) v.push_back(PropInfo(f)); } return v; }
static int PropOff(uintptr_t obj,const char* name){ for(auto& p:Props(ObjClass(obj))) if(p.name==name) return p.off; return -1; }
static uintptr_t PtrProp(uintptr_t obj,const char* name){ int o=PropOff(obj,name); return o<0?0:R<uintptr_t>(obj+o); }

static std::unordered_map<uintptr_t,int> gHealthOff;
static uintptr_t HealthComp(uintptr_t actor){
 uintptr_t cls=ObjClass(actor); if(!cls) return 0;
 auto it=gHealthOff.find(cls); int off;
 if(it!=gHealthOff.end()) off=it->second;
 else{ off=-1; for(auto& p:Props(cls)) if(p.type=="ObjectProperty"){ uintptr_t v=R<uintptr_t>(actor+p.off); if(Readable(v) && IsA(v,"MaiHealth")){ off=p.off; break; } }
  gHealthOff[cls]=off; }
 return off<0?0:R<uintptr_t>(actor+off);
}

static uintptr_t PlayerHealth(uintptr_t pawn){ uintptr_t h=PtrProp(pawn,"Health"); return (h && IsA(h,"SHCharacterPlayHealthComponent"))?h:0; }
static float PlayerMaxHealth(uintptr_t ph){ using F=float(__fastcall*)(uintptr_t); return ((F)(gBase+play::GetMaxHealth))(ph); }
static uintptr_t EnergySet(uintptr_t asc){ if(!asc) return 0; int o=PropOff(asc,"SpawnedAttributes"); if(o<0) return 0; uintptr_t a=R<uintptr_t>(asc+o); int n=R<int>(asc+o+8);
 for(int i=0;i<n && i<32;i++){ uintptr_t s=R<uintptr_t>(a+8ull*i); if(s && IsA(s,"CronosEnergySet")) return s; } return 0; }

struct Ctx{ uintptr_t world=0,gi=0,lp=0,pc=0,pawn=0,asc=0; };
static uintptr_t gEngine=0;
static Ctx Context(){
 Ctx c; if(!gEngine || !Readable(gEngine) || !IsA(gEngine,"GameEngine")) gEngine=FindFirst("GameEngine"); if(!gEngine) return c;
 uintptr_t vp=PtrProp(gEngine,"GameViewport"); if(!vp) return c;
 c.world=PtrProp(vp,"World"); if(!c.world) return c;
 c.gi=PtrProp(c.world,"OwningGameInstance"); if(!c.gi) return c;
 int o=PropOff(c.gi,"LocalPlayers"); if(o<0) return c;
 uintptr_t arr=R<uintptr_t>(c.gi+o); int num=R<int>(c.gi+o+8); if(!arr||num<1) return c;
 c.lp=R<uintptr_t>(arr); c.pc=PtrProp(c.lp,"PlayerController"); if(!c.pc) return c;
 using GPC=uintptr_t(__fastcall*)(uintptr_t,int); c.pawn=((GPC)(gBase+rva::GetPlayerCharacter))(c.pc,0);
 if(c.pawn){ using GA=uintptr_t(__fastcall*)(uintptr_t); c.asc=((GA)(gBase+rva::GetCharacterASC))(c.pawn); }
 return c;
}

static bool HasTag(uintptr_t asc,uint64_t tag){ uintptr_t itf=asc+0x118; uintptr_t vt=R<uintptr_t>(itf); using F=bool(__fastcall*)(uintptr_t,uint64_t); return ((F)R<uintptr_t>(vt+0x18))(itf,tag); }
static void ToggleTag(uintptr_t asc,uint64_t tag){ using T=void(__fastcall*)(uintptr_t,uint64_t); ((T)(gBase+rva::ToggleLooseTag))(asc,tag); }
static bool EnsureTag(uintptr_t asc,uint64_t tag,bool want){ if(!asc) return false; if(HasTag(asc,tag)!=want) ToggleTag(asc,tag); return HasTag(asc,tag)==want; }

static std::atomic<bool> wantGod{false}, wantOneHit{false}, wantAmmo{false}, wantEnergy{false};
static uintptr_t gProtData=0; static uint8_t gProtOrig=0; static float gCoolOrig=0;
static bool ammoOn=false;
static std::unordered_map<uintptr_t,uint8_t> godOrig, oneHitOrig;
static uintptr_t gEnemyClass=0; static std::vector<uintptr_t> gEnemies; static std::unordered_map<uintptr_t,bool> gClassIsEnemy;
static ULONGLONG gNextScan=0, gNextTick=0; static std::string gLastTick="нет";

static void ScanEnemies(){
 if(!gEnemyClass){ using SC=uintptr_t(__fastcall*)(); gEnemyClass=((SC)(gBase+rva::EnemyStaticClass))(); }
 gEnemies.clear(); if(!gEnemyClass) return;
 int n=ObjCount();
 for(int i=0;i<n;i++){ uintptr_t o=ObjAt(i); if(!o) continue; uintptr_t c=ObjClass(o); if(!c) continue;
  auto it=gClassIsEnemy.find(c); bool e; if(it==gClassIsEnemy.end()){ e=Derives(c,gEnemyClass); gClassIsEnemy[c]=e; } else e=it->second;
  if(e && !IsTemplate(o)) gEnemies.push_back(o); }
}
static void SetFlag(std::unordered_map<uintptr_t,uint8_t>& orig,uintptr_t comp,int off,bool on){
 if(!comp) return;
 if(on){ if(!orig.count(comp)) orig[comp]=R<uint8_t>(comp+off); Wr<uint8_t>(comp+off,1); }
 else{ auto it=orig.find(comp); if(it!=orig.end()){ Wr<uint8_t>(comp+off,it->second); orig.erase(it); } }
}
static void Release(std::unordered_map<uintptr_t,uint8_t>& orig,int off){ for(auto& kv:orig) if(Readable(kv.first)) Wr<uint8_t>(kv.first+off,kv.second); orig.clear(); }

static std::string TickV4(const Ctx& c); static bool AnyV4(); static bool WantNoReload(); static std::string TickV8(const Ctx& c); static void ReleaseV8();
static void Tick(){
 ULONGLONG now=GetTickCount64(); if(now<gNextTick) return; gNextTick=now+200;
 bool god=wantGod, one=wantOneHit, ammo=wantAmmo || WantNoReload();
 if(!god && !godOrig.empty()) Release(godOrig,mai::Immortal);
 if(!one && !oneHitOrig.empty()) Release(oneHitOrig,mai::OneHit);
 bool energy=wantEnergy;
 if(!god && gProtData){ if(Readable(gProtData)){ Wr<uint8_t>(gProtData+play::Protect,gProtOrig); Wr<float>(gProtData+play::Cooldown,gCoolOrig); } gProtData=0; }
 if(!god && !one && !ammo && !ammoOn && !energy && !AnyV4()) return;
 Ctx c=Context(); std::ostringstream st;
 if(god && c.pawn){ uintptr_t ph=PlayerHealth(c.pawn);
  if(ph){ uintptr_t da=R<uintptr_t>(ph+play::Data);
   if(da && da!=gProtData){ gProtData=da; gProtOrig=R<uint8_t>(da+play::Protect); gCoolOrig=R<float>(da+play::Cooldown); }
   if(da){ Wr<uint8_t>(da+play::Protect,1); Wr<float>(da+play::Cooldown,0.f); }
   float mx=PlayerMaxHealth(ph); if(mx>0 && R<float>(ph+play::HealthValue)<mx) Wr<float>(ph+play::HealthValue,mx); st<<"god=hp"<<R<float>(ph+play::HealthValue)<<"/"<<mx<<" "; }
  else st<<"god=нет компонента ";
  uintptr_t h=HealthComp(c.pawn); if(h) SetFlag(godOrig,h,mai::Immortal,true); }
 if(energy && c.pawn){ uintptr_t es=EnergySet(c.asc); if(es){ float mx=100.f; Wr<float>(es+0x60+8,mx); Wr<float>(es+0x60+0xC,mx); Wr<float>(es+0x80+8,R<float>(es+0x90+8)); Wr<float>(es+0x80+0xC,R<float>(es+0x90+0xC)); st<<"energy="<<mx<<" "; } else st<<"energy=нет набора "; }
 if(ammo!=ammoOn || (ammo && c.asc)){ uint64_t tag=R<uint64_t>(gBase+rva::TagUnlimitedAmmo); if(c.asc && tag){ EnsureTag(c.asc,tag,ammo); ammoOn=ammo; } }
 if(one){
  if(now>=gNextScan){ gNextScan=now+1500; ScanEnemies(); }
  int k=0; for(uintptr_t e:gEnemies){ if(!Readable(e)) continue; uintptr_t h=HealthComp(e); if(h){ SetFlag(oneHitOrig,h,mai::OneHit,true); k++; } }
  st<<"enemies="<<gEnemies.size()<<" flagged="<<k<<" ";
 }
 st<<TickV4(c)<<TickV8(c);
 gLastTick=st.str();
}

namespace v4 {
 constexpr uintptr_t AddSoul=0x55B6E50, RemoveSoul=0x55B6EB0;
 constexpr uintptr_t GetMagazineSize=0x56E36B0, GetAmmoInMagazine=0x56E3490, SetAmmoInMagazine=0x56E54F0;
}

static uint32_t FindNameIndex(const std::string& want){
 uint32_t cur=R<uint32_t>(gBase+rva::NamePool+8), curOff=R<uint32_t>(gBase+rva::NamePool+0xC);
 for(uint32_t b=0;b<=cur && b<8192;b++){
  uintptr_t blk=R<uintptr_t>(gBase+rva::NamePool+0x10+8ull*b); if(!blk) continue;
  uint32_t limit=(b==cur)?curOff:0x20000;
  for(uint32_t off=0;off+2<limit;){ uint16_t hdr=R<uint16_t>(blk+off); int len=hdr>>6; bool wide=hdr&1; if(len==0) break;
   if(!wide && (size_t)len==want.size()){ bool eq=true; for(int i=0;i<len && eq;i++) eq=R<char>(blk+off+2+i)==want[i]; if(eq) return (b<<16)|(off/2); }
   uint32_t sz=2+(wide?2*len:len); off+=(sz+1)&~1u; }
 }
 return 0xFFFFFFFF;
}
static uintptr_t AttrSet(uintptr_t asc,const char* cls){ if(!asc) return 0; int o=PropOff(asc,"SpawnedAttributes"); if(o<0) return 0; uintptr_t a=R<uintptr_t>(asc+o); int n=R<int>(asc+o+8);
 for(int i=0;i<n && i<32;i++){ uintptr_t s=R<uintptr_t>(a+8ull*i); if(s && IsA(s,cls)) return s; } return 0; }

struct AttrRule{ const char* set; const char* name; int mode; float value; };
struct Feature{ const char* key; std::vector<AttrRule> rules; };
static float gDamageMul=5.f, gSpeedMul=2.f;
static const char* WP[]={"GG","GG02","SG","SG02","MG","MG02","AG"};
static std::vector<Feature> BuildFeatures(){
 std::vector<Feature> f; auto per=[&](const char* suffix,int mode,float v,std::initializer_list<const char*> only={}){ std::vector<AttrRule> r; static std::deque<std::string> keep;
  for(const char* p:WP){ if(only.size()){ bool ok=false; for(const char* o:only) if(!strcmp(o,p)) ok=true; if(!ok) continue; } keep.push_back(std::string(p)+suffix); r.push_back({"CronosUpgradeSet",keep.back().c_str(),mode,v}); } return r; };
 auto cat=[](std::vector<AttrRule> a,const std::vector<AttrRule>& b){ a.insert(a.end(),b.begin(),b.end()); return a; };
 f.push_back({"damage",cat(per("BonusDamageMultiplier",2,0),{{"CronosSoulPerkSet","IncreasedMeleeDamage",2,0}})});
 f.push_back({"norecoil",cat(per("ShotRecoilReduction",0,1.f),per("ChargedShotRecoilReduction",0,1.f,{"GG","SG"}))});
 f.push_back({"accuracy",cat(cat(per("AimSwayReduction",0,1.f),per("AfterShotSpreadReduction",0,1.f)),per("BonusAccuracy",0,1.f,{"GG","GG02","MG","MG02","AG"}))});
 f.push_back({"reload",per("ReloadSpeedPlayRateMultiplier",0,5.f,{"GG","GG02","SG","SG02","MG","MG02"})});
 f.push_back({"inventory",{{"CronosUpgradeSet","InventorySize",1,12.f}}});
 return f;
}
static std::vector<Feature> gFeatures=BuildFeatures();
static std::unordered_map<std::string,bool> gWant=[]{ std::unordered_map<std::string,bool> m; for(auto& f:gFeatures) m[f.key]=false; for(const char* k:{"speed","noreload","firerate","charge","explosives","fabrication","noclip","nohit","autocode","noattack"}) m[k]=false; return m; }();
struct Orig{ uintptr_t set; int off; float base; };
static std::unordered_map<std::string,std::vector<Orig>> gOrig;
static std::string AttrRelease(const std::string& key){
 int k=0; for(auto& o:gOrig[key]) if(Readable(o.set)){ uintptr_t a=o.set+o.off; float b=R<float>(a+8), c=R<float>(a+0xC); Wr<float>(a+8,o.base); Wr<float>(a+0xC,o.base+(c-b)); k++; }
 gOrig.erase(key); return std::to_string(k);
}
static int AttrKeep(const Feature& f,uintptr_t asc){
 auto& orig=gOrig[f.key]; int k=0;
 for(auto& r:f.rules){ uintptr_t s=AttrSet(asc,r.set); if(!s) continue; int off=PropOff(s,r.name); if(off<0) continue; uintptr_t a=s+off;
  Orig* o=nullptr; for(auto& x:orig) if(x.set==s && x.off==off) o=&x;
  if(!o){ orig.push_back({s,off,R<float>(a+8)}); o=&orig.back(); }
  float target= r.mode==0? r.value : r.mode==1? o->base+r.value : (strcmp(f.key,"damage")==0? gDamageMul-1.f : r.value);
  float b=R<float>(a+8), c=R<float>(a+0xC);
  if(b!=target){ Wr<float>(a+0xC,target+(c-b)); Wr<float>(a+8,target); }
  k++; }
 return k;
}

static uintptr_t gSpeedComp=0; static float gSpeedOrig=1.f; static int gSpeedOff=-1;
static uintptr_t MoveComp(uintptr_t pawn){ uintptr_t m=PtrProp(pawn,"Movement"); if(!m) m=PtrProp(pawn,"CharacterMovement"); return m; }
static void SpeedRelease(){ if(gSpeedComp && gSpeedOff>=0 && Readable(gSpeedComp)) Wr<float>(gSpeedComp+gSpeedOff,gSpeedOrig); gSpeedComp=0; }

static std::vector<uintptr_t> gAmmoComps; static ULONGLONG gAmmoScan=0;
static void WeaponContainers(uintptr_t pawn,std::vector<uintptr_t>& out);
static std::string KeepMagazines(const Ctx& c){
 ULONGLONG now=GetTickCount64(); if(now>=gAmmoScan){ gAmmoScan=now+3000; std::vector<uintptr_t> cs; WeaponContainers(c.pawn,cs); gAmmoComps.clear(); for(uintptr_t k:cs){ uintptr_t a=PtrProp(k,"AmmoComponent"); if(a) gAmmoComps.push_back(a); } }
 using GI=int(__fastcall*)(uintptr_t); using SA=void(__fastcall*)(uintptr_t,int,bool);
 int filled=0;
 for(uintptr_t a:gAmmoComps){ if(!Readable(a)) continue;
  int size=((GI)(gBase+v4::GetMagazineSize))(a); if(size<=0 || size>100000) continue;
  int mag=((GI)(gBase+v4::GetAmmoInMagazine))(a);
  if(mag<size){ ((SA)(gBase+v4::SetAmmoInMagazine))(a,size,false); filled++; }}
 std::ostringstream o; o<<"noreload="<<gAmmoComps.size()<<"/"<<filled<<" "; return o.str();
}
static std::string TickV4(const Ctx& c){
 std::ostringstream st;
 for(auto& f:gFeatures){ bool on=gWant[f.key]; if(!on){ if(gOrig.count(f.key)) AttrRelease(f.key); continue; } if(c.asc){ int k=AttrKeep(f,c.asc); st<<f.key<<"="<<k<<" "; } }
 if(gWant["speed"] && c.pawn){ uintptr_t m=MoveComp(c.pawn); if(m){ if(m!=gSpeedComp){ SpeedRelease(); gSpeedOff=PropOff(m,"SpeedMultiplier"); gSpeedComp=m; gSpeedOrig=gSpeedOff>=0?R<float>(m+gSpeedOff):1.f; } if(gSpeedOff>=0) Wr<float>(m+gSpeedOff,gSpeedMul); st<<"speed="<<gSpeedMul<<" "; } }
 else if(!gWant["speed"] && gSpeedComp) SpeedRelease();
 if(gWant["noreload"] && c.pawn) st<<KeepMagazines(c);
 return st.str();
}
static bool WantNoReload(){ return gWant["noreload"]; }
static bool gV8Active=false;
static bool AnyV4(){ for(auto& kv:gWant) if(kv.second) return true; return !gOrig.empty() || gSpeedComp || gV8Active; }

static std::vector<std::string> TableRows(const std::string& table){
 std::vector<std::string> v; int n=ObjCount(); uintptr_t dt=0;
 for(int i=0;i<n;i++){ uintptr_t o=ObjAt(i); if(o && !IsTemplate(o) && ObjName(o)==table && IsA(o,"DataTable")){ dt=o; break; } }
 if(!dt) return v; uintptr_t data=R<uintptr_t>(dt+0x30); int num=R<int>(dt+0x38);
 for(int i=0;i<num && i<4096;i++){ uintptr_t e=data+0x18ull*i; std::string s=FNameStr(e); if(!s.empty()) v.push_back(s); }
 return v;
}

static uintptr_t FindFunc(uintptr_t obj,const char* name){ int d=0; for(uintptr_t c=ObjClass(obj);c && d<64;c=Super(c),d++){ int k=0; for(uintptr_t f=R<uintptr_t>(c+0x48);f && k<8192;f=R<uintptr_t>(f+0x28),k++) if(ObjName(f)==name) return f; } return 0; }
static uint64_t PropFlags(uintptr_t f){ return R<uint64_t>(f+0x38); }
struct Param{ Prop p; uint64_t flags; };
static std::vector<Param> Params(uintptr_t fn){ std::vector<Param> v; int k=0; for(uintptr_t f=R<uintptr_t>(fn+0x50);f && k<64;f=R<uintptr_t>(f+0x18),k++) v.push_back({PropInfo(f),PropFlags(f)}); return v; }
static int ParmsSize(uintptr_t fn){ return R<int>(fn+0x58); }
static bool CallFn(uintptr_t obj,uintptr_t fn,void* buf){ if(!obj||!fn) return false; __try{ gOrigPE((void*)obj,(void*)fn,buf); return true; }__except(EXCEPTION_EXECUTE_HANDLER){ return false; } }

static std::atomic<uintptr_t> gSkipDamageFn{0};

static std::unordered_map<std::string,uintptr_t> gFnCache;
static uintptr_t Fn(uintptr_t obj,const char* name){ std::string k=std::to_string(ObjClass(obj))+":"+name; auto it=gFnCache.find(k); if(it!=gFnCache.end()) return it->second; uintptr_t f=FindFunc(obj,name); gFnCache[k]=f; return f; }
static std::unordered_map<std::string,int> gOffCache;
static int Off(uintptr_t obj,const char* name){ std::string k=std::to_string(ObjClass(obj))+":"+name; auto it=gOffCache.find(k); if(it!=gOffCache.end()) return it->second; int o=PropOff(obj,name); gOffCache[k]=o; return o; }
static uintptr_t Ptr(uintptr_t obj,const char* name){ int o=Off(obj,name); return o<0?0:R<uintptr_t>(obj+o); }
struct Call{ uintptr_t obj,fn; std::vector<uint8_t> buf;
 Call(uintptr_t o,const char* name):obj(o),fn(o?Fn(o,name):0){ buf.assign((fn?ParmsSize(fn):0)+64,0); }
 int At(const char* p){ if(!fn) return -1; for(auto& pa:Params(fn)) if(pa.p.name==p) return pa.p.off; return -1; }
 template<class T> Call& Set(const char* p,T v){ int o=At(p); if(o>=0) memcpy(buf.data()+o,&v,sizeof(T)); return *this; }
 Call& Vec(const char* p,const double* v){ int o=At(p); if(o>=0) memcpy(buf.data()+o,v,24); return *this; }
 bool Go(){ return fn && CallFn(obj,fn,buf.data()); }
 template<class T> T Get(const char* p){ T v{}; int o=At(p); if(o>=0) memcpy(&v,buf.data()+o,sizeof(T)); return v; }
};
static uint64_t Name(const char* s){ static std::unordered_map<std::string,uint32_t> c; auto it=c.find(s); if(it!=c.end()) return it->second; uint32_t i=FindNameIndex(s); if(i!=0xFFFFFFFF) c[s]=i; return i==0xFFFFFFFF?0:i; }
static uintptr_t StaticObj(const char* defName){ static std::unordered_map<std::string,uintptr_t> c; auto it=c.find(defName); if(it!=c.end() && Readable(it->second)) return it->second;
 int n=ObjCount(); for(int i=0;i<n;i++){ uintptr_t o=ObjAt(i); if(o && ObjName(o)==defName){ c[defName]=o; return o; } } return 0; }

static uintptr_t ItemsComp(uintptr_t pawn){ uintptr_t i=Ptr(pawn,"Items"); return (i && IsA(i,"SHCharacterPlayItemsComponent"))?i:0; }
static int ItemCount(uintptr_t items,const char* row){ Call c(items,"GetItemCount"); c.Set<uint64_t>("ItemDataRowName",Name(row)); c.Go(); return c.Get<int>("ReturnValue"); }
static int ItemMax(uintptr_t items,const char* row){ Call c(items,"GetItemMaxCount"); c.Set<uint64_t>("ItemDataRowName",Name(row)); c.Go(); return c.Get<int>("ReturnValue"); }
static bool GiveItem(uintptr_t items,const char* row,int n){ if(!items||n<=0||!Name(row)) return false; Call c(items,"CollectItem"); c.Set<uint64_t>("ItemDataRowName",Name(row)).Set<int>("ItemQuantity",n); return c.Go(); }
static std::unordered_map<std::string,int> gKeepLevel;
static int KeepLevel(uintptr_t items,const char* row){ int have=ItemCount(items,row); int& lv=gKeepLevel[row]; if(have>lv) lv=have; if(have<lv){ GiveItem(items,row,lv-have); return 1; } return 0; }
static int TopUp(uintptr_t items,const char* row){ int mx=ItemMax(items,row), have=ItemCount(items,row); if(mx>0 && have<mx){ GiveItem(items,row,mx-have); return 1; } return 0; }

static void WeaponContainers(uintptr_t pawn,std::vector<uintptr_t>& out){
 uintptr_t wp=Ptr(pawn,"WeaponPlayerComponent"); if(!wp) return;
 for(const char* arr:{"WeaponModesContainers","AlternativeWeaponModesContainers"}){ int o=Off(wp,arr); if(o<0) continue; uintptr_t d=R<uintptr_t>(wp+o); int n=R<int>(wp+o+8); for(int i=0;i<n && i<32;i++){ uintptr_t k=R<uintptr_t>(d+8ull*i); if(k && Readable(k)) out.push_back(k); } }
 for(const char* one:{"FlamethrowerContainer","TrapMineContainer"}){ uintptr_t k=Ptr(wp,one); if(k) out.push_back(k); }
}
struct FOrig{ uintptr_t at; float v; };
static std::vector<FOrig> gFireOrig, gChargeOrig;
static void KeepFloat(std::vector<FOrig>& orig,uintptr_t at,float target){ bool have=false; for(auto& o:orig) if(o.at==at){ have=true; break; } if(!have) orig.push_back({at,R<float>(at)}); if(R<float>(at)!=target) Wr<float>(at,target); }
static float OrigOf(std::vector<FOrig>& orig,uintptr_t at){ for(auto& o:orig) if(o.at==at) return o.v; return R<float>(at); }
static void ReleaseFloats(std::vector<FOrig>& orig){ for(auto& o:orig) if(Readable(o.at)) Wr<float>(o.at,o.v); orig.clear(); }
static int ArrayElemSize(uintptr_t obj,const char* name){ int d=0; for(uintptr_t c=ObjClass(obj);c && d<64;c=Super(c),d++){ int k=0; for(uintptr_t f=R<uintptr_t>(c+0x50);f && k<4096;f=R<uintptr_t>(f+0x18),k++) if(FNameStr(f+0x20)==name){ uintptr_t inner=R<uintptr_t>(f+0x70); return inner?R<int>(inner+0x34):0; } } return 0; }
static std::string KeepWeapons(const Ctx& c,bool fire,bool charge){
 std::vector<uintptr_t> cs; WeaponContainers(c.pawn,cs); int nf=0,nc=0;
 for(uintptr_t k:cs){ uintptr_t fr=Ptr(k,"FireRateComponent"); if(!fr) continue;
  for(const char* b:{"TapFireBehavior","HoldFireBehavior"}){ uintptr_t fb=Ptr(fr,b); if(!fb) continue;
   if(fire) for(const char* f:{"FireCooldown","TimeBetweenShots","TimeBetweenBursts"}){ int o=Off(fb,f); if(o<0) continue; uintptr_t at=fb+o; KeepFloat(gFireOrig,at,OrigOf(gFireOrig,at)/4.f); nf++; }
   if(charge){ int o=Off(fb,"SettingsPerCharge"); if(o>=0){ uintptr_t d=R<uintptr_t>(fb+o); int n=R<int>(fb+o+8); int es=ArrayElemSize(fb,"SettingsPerCharge"); if(es<=0) es=0x40;
     for(int i=0;i<n && i<8;i++){ uintptr_t at=d+(uintptr_t)es*i; KeepFloat(gChargeOrig,at,0.05f); nc++; } } } }
  if(fire){ int o=Off(fr,"HoldFireStartDelayTime"); if(o>=0){ uintptr_t at=fr+o; KeepFloat(gFireOrig,at,0.02f); } } }
 if(!fire && !gFireOrig.empty()) ReleaseFloats(gFireOrig);
 if(!charge && !gChargeOrig.empty()) ReleaseFloats(gChargeOrig);
 std::ostringstream o; if(fire) o<<"firerate="<<nf<<" "; if(charge) o<<"charge="<<nc<<" "; return o.str();
}

struct NoClip{ bool on=false; uintptr_t pawn=0,move=0; double pos[3]={0,0,0}; float fly=600; ULONGLONG last=0; };
static NoClip gClip; static std::atomic<bool> gClipWant{false};
static bool Key(int vk){ return (GetAsyncKeyState(vk)&0x8000)!=0; }
static bool GameForeground(){ HWND w=GetForegroundWindow(); DWORD pid=0; if(w) GetWindowThreadProcessId(w,&pid); return pid==GetCurrentProcessId(); }
static void NoClipOff(){
 if(gClip.on && Readable(gClip.pawn)){
  if(gClip.move && Readable(gClip.move)){ int o=Off(gClip.move,"MaxFlySpeed"); if(o>=0) Wr<float>(gClip.move+o,gClip.fly); Call(gClip.move,"SetMovementMode").Set<uint8_t>("NewMovementMode",3).Go(); }
  Call(gClip.pawn,"SetActorEnableCollision").Set<uint8_t>("bNewActorEnableCollision",1).Go(); }
 gClip=NoClip();
}
static void NoClipFrame(const Ctx& c){
 if(!c.pawn){ return; }
 ULONGLONG now=GetTickCount64();
 if(!gClip.on || gClip.pawn!=c.pawn){
  NoClipOff(); gClip.on=true; gClip.pawn=c.pawn; gClip.move=Ptr(c.pawn,"CharacterMovement"); gClip.last=now;
  Call g(c.pawn,"K2_GetActorLocation"); g.Go(); int ro=g.At("ReturnValue"); if(ro>=0) memcpy(gClip.pos,g.buf.data()+ro,24);
  if(gClip.move){ int o=Off(gClip.move,"MaxFlySpeed"); if(o>=0){ gClip.fly=R<float>(gClip.move+o); Wr<float>(gClip.move+o,0.f); } Call(gClip.move,"SetMovementMode").Set<uint8_t>("NewMovementMode",5).Go(); }
  Call(c.pawn,"SetActorEnableCollision").Set<uint8_t>("bNewActorEnableCollision",0).Go();
 }
 double dt=(now-gClip.last)/1000.0; if(dt>0.1) dt=0.1; gClip.last=now;
 double pitch=0,yaw=0; int ro=Off(c.pc,"ControlRotation"); if(ro>=0){ pitch=R<double>(c.pc+ro); yaw=R<double>(c.pc+ro+8); }
 const double k=3.14159265358979/180.0; double cp=cos(pitch*k), sp=sin(pitch*k), cy=cos(yaw*k), sy=sin(yaw*k);
 double f[3]={cp*cy,cp*sy,sp}, r[3]={-sy,cy,0};
 bool fg=GameForeground();
 double mf=fg?(double)(Key('W')-Key('S')):0, mr=fg?(double)(Key('D')-Key('A')):0, mu=fg?(double)(Key(VK_SPACE)-Key(VK_CONTROL)):0;
 double speed=(fg && Key(VK_SHIFT))?2400.0:800.0;
 for(int i=0;i<3;i++) gClip.pos[i]+=(f[i]*mf+r[i]*mr+(i==2?mu:0.0))*speed*dt;
 Call s(c.pawn,"K2_SetActorLocation"); s.Vec("NewLocation",gClip.pos).Set<uint8_t>("bSweep",0).Set<uint8_t>("bTeleport",1); s.Go();
 if(gClip.move){ int vo=Off(gClip.move,"Velocity"); if(vo>=0) for(int i=0;i<3;i++) Wr<double>(gClip.move+vo+8*i,0.0); }
}
static Ctx gFrameCtx; static ULONGLONG gFrameCtxAt=0, gFrameLast=0;
static void FrameTick(){
 if(!gClipWant && !gClip.on) return;
 ULONGLONG now=GetTickCount64(); if(now-gFrameLast<5) return; gFrameLast=now;
 if(!gClipWant){ NoClipOff(); return; }
 if(now-gFrameCtxAt>500 || !Readable(gFrameCtx.pawn)){ gFrameCtx=Context(); gFrameCtxAt=now; }
 NoClipFrame(gFrameCtx);
}

static int TagCount(uintptr_t asc,const char* tag){ Call c(asc,"GetGameplayTagCount"); c.Set<uint64_t>("GameplayTag",Name(tag)); c.Go(); return c.Get<int>("ReturnValue"); }
static void TagSet(uintptr_t asc,const char* tag,bool on){ if(!asc||!Name(tag)) return; if(on){ if(TagCount(asc,tag)<=0) Call(asc,"AddUniqueGameplayTag").Set<uint64_t>("Tag",Name(tag)).Go(); } else if(TagCount(asc,tag)>0) Call(asc,"RemoveGameplayTagCompletely").Set<uint64_t>("Tag",Name(tag)).Go(); }

static uintptr_t gHitComp=0; static uint8_t gHitOrig=1;
static uintptr_t gInvulAsc=0;
static void NoAttackOff(){ if(gInvulAsc && Readable(gInvulAsc)) TagSet(gInvulAsc,"State.Invulnerable",false); gInvulAsc=0; }
static std::string NoAttackKeep(const Ctx& c){ if(!c.asc) return ""; if(gInvulAsc && gInvulAsc!=c.asc && Readable(gInvulAsc)) TagSet(gInvulAsc,"State.Invulnerable",false); gInvulAsc=c.asc; TagSet(c.asc,"State.Invulnerable",true); return "noattack=1 "; }
static bool BoolProp(uintptr_t obj,const char* name,int& at,uint8_t& mask){ int d=0; for(uintptr_t c=ObjClass(obj);c && d<64;c=Super(c),d++){ int k=0; for(uintptr_t f=R<uintptr_t>(c+0x50);f && k<4096;f=R<uintptr_t>(f+0x18),k++) if(FNameStr(f+0x20)==name){ at=R<int>(f+0x44)+R<uint8_t>(f+0x71); mask=R<uint8_t>(f+0x72); return mask!=0; } } return false; }
static uintptr_t gNoDmgPawn=0; static bool gNoDmgOrig=true;
static void NoDamageOff(){ if(gNoDmgPawn && Readable(gNoDmgPawn)){ int at; uint8_t m; if(BoolProp(gNoDmgPawn,"bCanBeDamaged",at,m)){ uint8_t v=R<uint8_t>(gNoDmgPawn+at); Wr<uint8_t>(gNoDmgPawn+at,gNoDmgOrig?(v|m):(v&~m)); } } gNoDmgPawn=0; gSkipDamageFn=0; }
static std::string NoDamageKeep(const Ctx& c){
 int at; uint8_t m; if(BoolProp(c.pawn,"bCanBeDamaged",at,m)){ uint8_t v=R<uint8_t>(c.pawn+at); if(gNoDmgPawn!=c.pawn){ gNoDmgPawn=c.pawn; gNoDmgOrig=(v&m)!=0; } if(v&m) Wr<uint8_t>(c.pawn+at,v&~m); }
 uintptr_t ph=PlayerHealth(c.pawn); if(ph){ gSkipDamageFn=Fn(ph,"ReceiveDamage"); float mx=PlayerMaxHealth(ph); if(mx>0 && R<float>(ph+play::HealthValue)<mx) Wr<float>(ph+play::HealthValue,mx); }
 int removed=0;
 if(c.asc){ for(const char* t:{"Gameplay.StatusEffects.Burning","Gameplay.StatusEffects.Burning.Active","Gameplay.StatusEffects.AcidPuddle","Gameplay.StatusEffects.Grabbed"}){ uint64_t id=Name(t); if(!id) continue;
   if(TagCount(c.asc,t)>0){ static uint64_t tagbuf[1]; tagbuf[0]=id; uint8_t cont[0x20]={0}; *(uint64_t**)cont=tagbuf; *(int*)(cont+8)=1; *(int*)(cont+12)=1;
    Call r(c.asc,"RemoveActiveEffectsWithGrantedTags"); int to=r.At("Tags"); if(to>=0){ memcpy(r.buf.data()+to,cont,0x20); r.Go(); removed+=r.Get<int>("ReturnValue"); }
    Call r2(c.asc,"RemoveActiveEffectsWithTags"); to=r2.At("Tags"); if(to>=0){ memcpy(r2.buf.data()+to,cont,0x20); r2.Go(); removed+=r2.Get<int>("ReturnValue"); } } } }
 return "nodmg="+std::to_string(removed)+" ";
}
struct ChanOrig{ uintptr_t comp; uint8_t ch, resp; }; static std::vector<ChanOrig> gChanOrig;
static const uint8_t kHitChannels[]={9,10,11,12,13,26,28};
static uint8_t RespOf(uintptr_t comp,uint8_t ch){ Call g(comp,"GetCollisionResponseToChannel"); g.Set<uint8_t>("Channel",ch).Go(); return g.Get<uint8_t>("ReturnValue"); }
static void SetResp(uintptr_t comp,uint8_t ch,uint8_t r){ Call(comp,"SetCollisionResponseToChannel").Set<uint8_t>("Channel",ch).Set<uint8_t>("NewResponse",r).Go(); }
static void ChannelsKeep(uintptr_t pawn){
 for(const char* n:{"CapsuleComponent","Mesh","PhysicsCollision"}){ uintptr_t c=Ptr(pawn,n); if(!c) continue;
  for(uint8_t ch:kHitChannels){ bool have=false; for(auto& o:gChanOrig) if(o.comp==c && o.ch==ch){ have=true; break; }
   uint8_t r=RespOf(c,ch); if(!have){ gChanOrig.push_back({c,ch,r}); } if(r!=0) SetResp(c,ch,0); } } }
static void ChannelsRelease(){ for(auto& o:gChanOrig) if(Readable(o.comp)) SetResp(o.comp,o.ch,o.resp); gChanOrig.clear(); }
static void NoHitOff(){ if(gHitComp && Readable(gHitComp)) Call(gHitComp,"SetCollisionEnabled").Set<uint8_t>("NewType",gHitOrig).Go(); gHitComp=0; ChannelsRelease(); NoDamageOff(); }
static std::string NoHitKeep(const Ctx& c){
 uintptr_t h=Ptr(c.pawn,"HitCollision"); if(!h) return "nohit=нет коллизии ";
 if(h!=gHitComp){ NoHitOff(); Call g(h,"GetCollisionEnabled"); g.Go(); gHitOrig=g.Get<uint8_t>("ReturnValue"); if(gHitOrig==0) gHitOrig=1; gHitComp=h; }
 Call g(h,"GetCollisionEnabled"); g.Go(); if(g.Get<uint8_t>("ReturnValue")!=0) Call(h,"SetCollisionEnabled").Set<uint8_t>("NewType",0).Go();
 ChannelsKeep(c.pawn); return "nohit="+std::to_string(gChanOrig.size())+" "+NoDamageKeep(c);
}

static std::string FStr(uintptr_t at){ uintptr_t d=R<uintptr_t>(at); int n=R<int>(at+8); std::string s; for(int i=0;i<n-1 && i<64;i++){ wchar_t ch=R<wchar_t>(d+2*i); if(ch<128) s+=(char)ch; } return s; }
static uintptr_t gPadClass=0; static std::vector<uintptr_t> gPads; static ULONGLONG gPadScan=0; static std::unordered_map<uintptr_t,std::string> gPadShown;
static bool SetTextBlock(uintptr_t tb,const std::string& text){
 uintptr_t lib=StaticObj("Default__KismetTextLibrary"); if(!lib||!tb) return false;
 static wchar_t wbuf[64]; int n=0; for(char ch:text){ if(n<62) wbuf[n++]=(wchar_t)ch; } wbuf[n]=0;
 Call conv(lib,"Conv_StringToText"); int so=conv.At("InString"); if(so<0) return false;
 uintptr_t* fs=(uintptr_t*)(conv.buf.data()+so); fs[0]=(uintptr_t)wbuf; ((int*)(conv.buf.data()+so+8))[0]=n+1; ((int*)(conv.buf.data()+so+8))[1]=n+1;
 if(!conv.Go()) return false; int ro=conv.At("ReturnValue");
 Call st(tb,"SetText"); int to=st.At("InText"); if(to<0||ro<0) return false; memcpy(st.buf.data()+to,conv.buf.data()+ro,0x18); return st.Go();
}
static std::string AutoCode(const Ctx& c){
 ULONGLONG now=GetTickCount64();
 if(now>=gPadScan){ gPadScan=now+700; gPads.clear(); if(!gPadClass) gPadClass=StaticObj("BTPinPadWidget");
  static std::unordered_map<uintptr_t,bool> isPad;
  if(gPadClass){ int n=ObjCount(); for(int i=0;i<n;i++){ uintptr_t o=ObjAt(i); if(!o) continue; uintptr_t cl=ObjClass(o); if(!cl) continue; auto it=isPad.find(cl); bool y; if(it==isPad.end()){ y=Derives(cl,gPadClass); isPad[cl]=y; } else y=it->second; if(y && !IsTemplate(o)) gPads.push_back(o); } } }
 std::string shown;
 for(uintptr_t w:gPads){ if(!Readable(w)) continue; int co=Off(w,"CorrectCodeString"); if(co<0) continue; std::string code=FStr(w+co); if(code.empty()) continue;
  uintptr_t box=Ptr(w,"UnlockCodeHintContainer"), tb=Ptr(w,"UnlockCodeHintTextBlock");
  if(tb && gPadShown[w]!=code){ if(SetTextBlock(tb,code)) gPadShown[w]=code; }
  if(box){ Call g(box,"GetVisibility"); g.Go(); uint8_t v=g.Get<uint8_t>("ReturnValue"); if(v==1||v==2) Call(box,"SetVisibility").Set<uint8_t>("InVisibility",4).Go(); }
  if(tb){ Call g(tb,"GetVisibility"); g.Go(); uint8_t v=g.Get<uint8_t>("ReturnValue"); if(v==1||v==2) Call(tb,"SetVisibility").Set<uint8_t>("InVisibility",4).Go(); }
  shown=code; }
 return shown.empty()?"autocode=- ":"autocode="+shown+" ";
}

static std::string TickV8(const Ctx& c){
 std::ostringstream st; bool any=false;
 gClipWant=gWant["noclip"]; if(gClipWant) any=true;
 if(gWant["nohit"] && c.pawn){ st<<NoHitKeep(c); any=true; } else if(gHitComp) NoHitOff();
 if(gWant["noattack"] && c.pawn){ st<<NoAttackKeep(c); any=true; } else if(gInvulAsc) NoAttackOff();
 bool fire=gWant["firerate"], charge=gWant["charge"];
 if(c.pawn && (fire||charge||!gFireOrig.empty()||!gChargeOrig.empty())){ st<<KeepWeapons(c,fire,charge); if(fire||charge) any=true; }
 if(gWant["autocode"]){ st<<AutoCode(c); any=true; } else if(!gPadShown.empty()) gPadShown.clear();
 uintptr_t items=c.pawn?ItemsComp(c.pawn):0;
 if(items && gWant["fabrication"]){ int k=TopUp(items,"Resource01")+TopUp(items,"Resource02"); st<<"craft=+"<<k<<" "; any=true; }
 if(items && gWant["explosives"]){ int k=KeepLevel(items,"FlamethrowerAmmo")+KeepLevel(items,"MineAmmo"); st<<"explosives=+"<<k<<" "; any=true; } else gKeepLevel.clear();
 gV8Active=any || gHitComp || !gChanOrig.empty() || gInvulAsc || !gFireOrig.empty() || !gChargeOrig.empty() || gClip.on;
 return st.str();
}

static std::string Exec(const std::string& line){
 std::istringstream in(line); std::string cmd; in>>cmd;
 try{
  if(cmd=="hello"){ Ctx c=Context(); uintptr_t h=c.pawn?HealthComp(c.pawn):0;
   return "ok bridge=1 objects="+std::to_string(ObjCount())+" pc="+Hex(c.pc)+" pawn="+Hex(c.pawn)+" asc="+Hex(c.asc)+" world="+(c.world?ObjName(c.world):"-")+
    " pawnClass="+(c.pawn?ObjName(ObjClass(c.pawn)):"-")+" health="+(h?ObjName(ObjClass(h))+":"+std::to_string(R<float>(h+mai::Health)):"-"); }
  if(cmd=="set"){ std::string n; int v=0; in>>n>>v; bool on=v!=0;
   bool known=gWant.count(n)>0;
   if(n=="god") wantGod=on; else if(n=="onehit") wantOneHit=on; else if(n=="ammo") wantAmmo=on; else if(n=="energy") wantEnergy=on; else if(known) gWant[n]=on; else return "err неизвестная функция "+n;
   gNextTick=0; Tick(); return "ok "+n+"="+(on?"1":"0")+" "+gLastTick; }
  if(cmd=="state"){ std::ostringstream o; o<<"ok god="<<wantGod<<" onehit="<<wantOneHit<<" ammo="<<wantAmmo<<" energy="<<wantEnergy; for(auto& kv:gWant) o<<" "<<kv.first<<"="<<kv.second; o<<" | "<<gLastTick; return o.str(); }
  if(cmd=="mul"){ std::string n; float v=0; in>>n>>v; if(v<=0) return "err значение"; if(n=="damage") gDamageMul=v; else if(n=="speed") gSpeedMul=v; else return "err параметр"; gNextTick=0; Tick(); return "ok "+n+"="+std::to_string(v); }
  if(cmd=="rows"){ std::string t; in>>t; auto r=TableRows(t); std::ostringstream o; o<<"ok "<<r.size(); for(auto& s:r) o<<"\n"<<s; return o.str(); }
  if(cmd=="soul"){ std::string op,name; in>>op>>name; Ctx c=Context(); if(!c.pawn) return "err нет игрока"; uint32_t idx=FindNameIndex(name); if(idx==0xFFFFFFFF) return "err нет такой души: "+name;
   using F=void(__fastcall*)(uintptr_t,uint64_t); if(op=="add") ((F)(gBase+v4::AddSoul))(c.pc,idx); else if(op=="remove") ((F)(gBase+v4::RemoveSoul))(c.pc,idx); else return "err операция";
   return "ok "+op+" "+name; }
  if(cmd=="cheat"){
   std::string n; in>>n; Ctx c=Context(); if(!c.pawn) return "err нет игрока (загрузите сохранение)";
   struct{const char* n; uintptr_t r;} t[]={{"GiveAllEquipment",rva::GiveAllEquipment},{"GiveAllItems",rva::GiveAllItems},{"GiveAllResources",rva::GiveAllResources},{"GiveAmmo",rva::GiveAmmo},{"KillAll",rva::KillAll},{"UnlockAllUpgrades",rva::UnlockAllUpgrades}};
   for(auto& e:t) if(n==e.n){ using F=void(__fastcall*)(uintptr_t); ((F)(gBase+e.r))(c.pc); return "ok "+n; }
   return "err неизвестная функция "+n;
  }
  if(cmd=="heal"){ Ctx c=Context(); if(!c.pawn) return "err нет игрока"; uintptr_t ph=PlayerHealth(c.pawn); if(!ph) return "err нет компонента здоровья";
   float mx=PlayerMaxHealth(ph); if(mx<=0) return "err максимум здоровья не получен"; Wr<float>(ph+play::HealthValue,mx); return "ok "+std::to_string(mx); }
  if(cmd=="give"){ std::string row; int n=0; in>>row>>n; Ctx c=Context(); uintptr_t items=c.pawn?ItemsComp(c.pawn):0; if(!items) return "err нет игрока (загрузите сохранение)";
   if(row=="Resources"){ int k=TopUp(items,"Resource01")+TopUp(items,"Resource02"); return "ok Resources +"+std::to_string(k); }
   if(!Name(row.c_str())) return "err нет предмета "+row; int before=ItemCount(items,row.c_str()); GiveItem(items,row.c_str(),n); int after=ItemCount(items,row.c_str());
   return "ok "+row+" "+std::to_string(before)+" -> "+std::to_string(after); }
 }catch(...){ return "err исключение"; }
 return "err неизвестная команда";
}

struct Job{ std::string in,out; HANDLE done; };
static std::mutex gLock; static std::deque<Job*> gJobs; static std::atomic<int> gPending{0};
static thread_local bool tInside=false;
static void DoJob(Job* j){ j->out=Exec(j->in); }
static bool TryJob(Job* j){ __try{ DoJob(j); return true; }__except(EXCEPTION_EXECUTE_HANDLER){ return false; } }
static void RunJobs(){
 std::deque<Job*> run; { std::lock_guard<std::mutex> g(gLock); run.swap(gJobs); gPending=0; }
 for(Job* j:run){ if(!TryJob(j)) j->out="err сбой при выполнении (команда отменена)"; SetEvent(j->done); }
}
static void SafeFrame(){ __try{ FrameTick(); }__except(EXCEPTION_EXECUTE_HANDLER){ gClip=NoClip(); } }
static void SafeTick(){ __try{ Tick(); }__except(EXCEPTION_EXECUTE_HANDLER){ gLastTick="сбой в такте"; } }
static void __fastcall HookPE(void* obj,void* fn,void* params){
 uintptr_t f=(uintptr_t)fn;
 if(f && f==gSkipDamageFn.load(std::memory_order_relaxed)){ if(params) memset((uint8_t*)params+16,0,4); return; }
 if(!tInside && GetCurrentThreadId()==gGameThread){
  tInside=true;
  if(gPending.load(std::memory_order_relaxed)) RunJobs();
  SafeTick();
  SafeFrame();
  tInside=false;
 }
 gOrigPE(obj,fn,params);
}
static std::string RunOnGame(const std::string& s){
 Job j{s,"",CreateEventW(nullptr,TRUE,FALSE,nullptr)}; { std::lock_guard<std::mutex> g(gLock); gJobs.push_back(&j); gPending=1; }
 if(WaitForSingleObject(j.done,5000)!=WAIT_OBJECT_0){
  { std::lock_guard<std::mutex> g(gLock); for(auto it=gJobs.begin();it!=gJobs.end();++it) if(*it==&j){ gJobs.erase(it); CloseHandle(j.done); return "err игра не ответила (пауза или загрузка)"; } }
  WaitForSingleObject(j.done,INFINITE); }
 CloseHandle(j.done); return j.out;
}

static bool InstallHook(){
 uint8_t* pe=(uint8_t*)(gBase+rva::ProcessEvent);

 bool takeover=pe[0]==0xFF && pe[1]==0x25 && *(uint32_t*)(pe+2)==0;
 if(!takeover && memcmp(pe,PEPrologue,19)!=0) return false;
 gTramp=(uint8_t*)VirtualAlloc(nullptr,64,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE); if(!gTramp) return false;
 memcpy(gTramp,PEPrologue,19); uint8_t jb[14]={0xFF,0x25,0,0,0,0}; uintptr_t back=(uintptr_t)pe+19; memcpy(jb+6,&back,8); memcpy(gTramp+19,jb,14);
 gOrigPE=(PEFn)gTramp;
 if(takeover){ uintptr_t to=(uintptr_t)&HookPE; DWORD old; VirtualProtect(pe,19,PAGE_EXECUTE_READWRITE,&old); InterlockedExchange64((volatile LONG64*)(pe+6),(LONG64)to); VirtualProtect(pe,19,old,&old); FlushInstructionCache(GetCurrentProcess(),pe,19); return true; }
 uint8_t patch[19]; uintptr_t to=(uintptr_t)&HookPE; patch[0]=0xFF; patch[1]=0x25; memset(patch+2,0,4); memcpy(patch+6,&to,8); memset(patch+14,0x90,5);
 DWORD old; VirtualProtect(pe,19,PAGE_EXECUTE_READWRITE,&old); memcpy(pe,patch,19); VirtualProtect(pe,19,old,&old); FlushInstructionCache(GetCurrentProcess(),pe,19);
 return true;
}

static DWORD WINAPI ClientThread(LPVOID arg){
 HANDLE h=(HANDLE)arg; bool owner=false; std::vector<char> buf(65536);
 for(;;){ DWORD n=0; if(!ReadFile(h,buf.data(),(DWORD)buf.size()-1,&n,nullptr)||n==0) break; std::string req(buf.data(),n);
  if(req.rfind("set ",0)==0 || req.rfind("mul ",0)==0 || req.rfind("cheat ",0)==0) owner=true;
  std::string resp= !gHooked? "err перехват не установлен (другая версия игры)" : (req=="ping"? "ok" : RunOnGame(req));
  DWORD w; if(!WriteFile(h,resp.data(),(DWORD)resp.size(),&w,nullptr)) break; }
 if(owner){ wantGod=false; wantOneHit=false; wantAmmo=false; wantEnergy=false; for(auto& kv:gWant) kv.second=false; }
 DisconnectNamedPipe(h); CloseHandle(h); return 0;
}
static DWORD WINAPI PipeThread(LPVOID){
 for(;;){
  HANDLE h=CreateNamedPipeW(L"\\\\.\\pipe\\CronosTrainerBridge",PIPE_ACCESS_DUPLEX,PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,PIPE_UNLIMITED_INSTANCES,1<<20,1<<20,0,nullptr);
  if(h==INVALID_HANDLE_VALUE){ Sleep(1000); continue; }
  if(ConnectNamedPipe(h,nullptr)||GetLastError()==ERROR_PIPE_CONNECTED) CloseHandle(CreateThread(nullptr,0,ClientThread,h,0,nullptr));
  else CloseHandle(h);
 }
}

static DWORD FindGameThread(){
 DWORD pid=GetCurrentProcessId(), best=0; HWND w=nullptr;
 while((w=FindWindowExW(nullptr,w,nullptr,nullptr))){ DWORD p; DWORD t=GetWindowThreadProcessId(w,&p); if(p==pid && IsWindowVisible(w)){ best=t; break; } }
 return best;
}

static DWORD WINAPI Init(LPVOID){
 gBase=(uintptr_t)GetModuleHandleW(nullptr);
 auto dos=(IMAGE_DOS_HEADER*)gBase; auto nt=(IMAGE_NT_HEADERS64*)(gBase+dos->e_lfanew);
 if(nt->FileHeader.TimeDateStamp==rva::Stamp && nt->OptionalHeader.SizeOfImage==rva::Image){
  for(int i=0;i<600 && !gGameThread;i++){ gGameThread=FindGameThread(); if(!gGameThread) Sleep(500); }
  gHooked=gGameThread && InstallHook();
 }
 PipeThread(nullptr); return 0;
}

BOOL APIENTRY DllMain(HMODULE m,DWORD r,LPVOID){ if(r==DLL_PROCESS_ATTACH){ DisableThreadLibraryCalls(m); CloseHandle(CreateThread(nullptr,0,Init,nullptr,0,nullptr)); } return TRUE; }

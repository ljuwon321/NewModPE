#include <dlfcn.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <memory>
#include <vector>

#define LOG_TAG "newmodpe"

#include "log.h"
#include "access.h"
#include "hook.h"

#define MCPE_DATA_PATH "/sdcard/games/com.mojang/NewModPEDex.dex"
#define MCPE_DATA_PATH_DUMMY "/data/data/net.zhuoweizhang.mcpelauncher.pro/files/"

static bool dexLoaded = false;

extern JavaVM* bl_JavaVM;
extern jclass bl_scriptmanager_class;

static jclass ScriptState;
static jclass ScriptableObject;
static jclass N_Player;

class BinaryStream;
class EntityDefinitionGroup;
class EntityDefinitionIdentifier;
class Level;
class NetEventCallback;
class NetworkHandler;
class NetworkIdentifier;

// Size : 12
class Packet {
	public:
	// void** vtable; // 0
	char filler1[8]; // 4

	public:
	virtual ~Packet();
	virtual int getId() const = 0;
	virtual void write(BinaryStream&) const = 0;
	virtual void read(BinaryStream&) = 0;
	virtual void handle(NetworkIdentifier const&, NetEventCallback const&) const = 0;
	virtual bool disallowBatching();
};

// Size : 4
class MinecraftPackets {
	public:
	Packet* retval; // 0

	public:
	Packet* createPacket(int);
};

typedef long long EntityRuntimeID;
typedef long long EntityUniqueID;

// Size : 24
class InteractPacket : public Packet {
	public:
	unsigned char type; // 12
	EntityRuntimeID runtimeID; // 16

	public:
	virtual ~InteractPacket();
	virtual int getId() const;
	virtual void write(BinaryStream&) const;
	virtual void read(BinaryStream&);
	virtual void handle(NetworkIdentifier const&, NetEventCallback const&) const;
};

class Entity {
	public:
	float distanceTo(Entity const&) const;
	Level* getLevel();
	Level* getLevel() const;
	EntityRuntimeID getRuntimeID() const;
	EntityUniqueID getUniqueID() const;
	bool hasRuntimeID() const;
};

class Mob : public Entity {
	public:
	virtual ~Mob();
	Mob(EntityDefinitionGroup&, EntityDefinitionIdentifier const&);
	Mob(Level&);
};

class Player : public Mob {
    public:
    void attack(Entity&);
};

enum class EntityType {
	PLAYER = 0x100 | 63
};

class EntityClassTree {
	public:
	static bool isInstanceOf(Entity const&, EntityType);
};

class BatchedPacketSender {
	public:
	virtual ~BatchedPacketSender();
	virtual void send(Packet const&);
	virtual void send(NetworkIdentifier const&, Packet const&);
	virtual void sendBroadcast(NetworkIdentifier const&, Packet const&);
	virtual void sendBroadcast(Packet const&);
	virtual void flush(NetworkIdentifier const&);

	BatchedPacketSender(NetworkHandler&);
	/* BatchPacket* */ void _getBatch(NetworkIdentifier const&);
	bool _playerExists(NetworkIdentifier const&) const;
	void _queuePacket(NetworkIdentifier const&, Packet const&);
	void addLoopbackCallback(NetEventCallback &);
	void removeLoopbackCallback(NetEventCallback &);
	void setPlayerList(std::vector<std::unique_ptr<Player>> const*);
	void update();
};

typedef BatchedPacketSender PacketSender;

class Minecraft {
	public:
	PacketSender* getPacketSender();
};

class MinecraftClient {
	public:
	Minecraft* getServer();
	void onTick(int, int);
};

static void (*MinecraftClient$onPlayerLoaded_real)(MinecraftClient*, Player*);
static void MinecraftClient$onPlayerLoaded(MinecraftClient* client, Player* player) {
    if (!dexLoaded) {
        MinecraftClient$onPlayerLoaded_real(client, player);
        return;
    }

    JNIEnv* env;
    int attachStatus = bl_JavaVM->GetEnv((void**) &env, JNI_VERSION_1_2);
    if (attachStatus == JNI_EDETACHED)
        bl_JavaVM->AttachCurrentThread(&env, NULL);
    jclass listCls = env->FindClass("java/util/List");
    jobject scripts = env->GetStaticObjectField(bl_scriptmanager_class, env->GetStaticFieldID(bl_scriptmanager_class, "scripts", "Ljava/util/List;"));
    jint size = env->CallIntMethod(scripts, env->GetMethodID(listCls, "size", "()I"));
    jmethodID listGet = env->GetMethodID(listCls, "get", "(I)Ljava/lang/Object;");
    jfieldID getScope = env->GetFieldID(ScriptState, "scope", "Lorg/mozilla/javascript/Scriptable;");
    jmethodID hasProperty = env->GetStaticMethodID(ScriptableObject, "hasProperty", "(Lorg/mozilla/javascript/Scriptable;Ljava/lang/String;)Z");
    jmethodID defineClass = env->GetStaticMethodID(ScriptableObject, "defineClass", "(Lorg/mozilla/javascript/Scriptable;Ljava/lang/Class;Z)V");

    for (int i = 0; i < size; i++) {
        jobject scope = env->GetObjectField(env->CallObjectMethod(scripts, listGet, i), getScope);
        if (!env->CallStaticBooleanMethod(ScriptableObject, hasProperty, scope, env->NewStringUTF("N_Player")))
            env->CallStaticVoidMethod(ScriptableObject, defineClass, scope, N_Player, true);
    }

    if (attachStatus == JNI_EDETACHED)
        bl_JavaVM->DetachCurrentThread();
    MinecraftClient$onPlayerLoaded_real(client, player);
}

static struct {
    bool enable;
    bool hitMob;
    int distance;
} roundAttackConfig = { false, false, 12 };

JNIEXPORT void JNICALL nativeSetRoundAttack(JNIEnv* env, jclass clazz, jboolean enable, jboolean hitMob, jint distance) {
    roundAttackConfig.enable = enable;
    roundAttackConfig.hitMob = hitMob;
    roundAttackConfig.distance = distance;
}

static std::vector<Mob*> mobs;

static Mob* (*Mob$Mob_1_real)(Mob*, EntityDefinitionGroup*, EntityDefinitionIdentifier const*);
static Mob* Mob$Mob_1(Mob* mob, EntityDefinitionGroup* group, EntityDefinitionIdentifier const* identifier) {
	Mob$Mob_1_real(mob, group, identifier);
	if (std::find(mobs.begin(), mobs.end(), mob) == mobs.end()) mobs.push_back(mob);
	return mob;
}

static Mob* (*Mob$Mob_2_real)(Mob*, Level*);
static Mob* Mob$Mob_2(Mob* mob, Level* level) {
	Mob$Mob_2_real(mob, level);
	if (std::find(mobs.begin(), mobs.end(), mob) == mobs.end()) mobs.push_back(mob);
	return mob;
}

static Mob* (*Mob$dMob_real)(Mob*);
static Mob* Mob$dMob(Mob* mob) {
	mobs.erase(std::remove(mobs.begin(), mobs.end(), mob), mobs.end());
	Mob$dMob_real(mob);
	return mob;
}

static void (*MinecraftClient$onTick_real)(MinecraftClient*, int, int);
static void MinecraftClient$onTick(MinecraftClient* client, int idk1, int idk2) {
    MinecraftClient$onTick_real(client, idk1, idk2);
    if (!roundAttackConfig.enable)
        return;
    Player* thisPlayer = access(client, Player*, 0x120);
    if (thisPlayer == NULL)
        return;
    MinecraftPackets packetProvider;
    for (Mob* const& target : mobs)
        if (target != thisPlayer
                && target != NULL
                && (roundAttackConfig.hitMob || EntityClassTree::isInstanceOf(*target, EntityType::PLAYER))
                && target->distanceTo(*thisPlayer) <= roundAttackConfig.distance) {
                thisPlayer->attack(*target);
                InteractPacket* pk = static_cast<InteractPacket*>(packetProvider.createPacket(0x22));
                pk->type = 2;
                pk->runtimeID = access(target, EntityRuntimeID, 0xC60);
                client->getServer()->getPacketSender()->send(*pk);
        }
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    FILE* dexFile;

    if ((dexFile = fopen(MCPE_DATA_PATH, "r")) != NULL) {
        fclose(dexFile);
        dexLoaded = true;
        JNIEnv* env;
        int attachStatus = bl_JavaVM->GetEnv((void**) &env, JNI_VERSION_1_2);
        if (attachStatus == JNI_EDETACHED)
            bl_JavaVM->AttachCurrentThread(&env, NULL);
        jclass classCls = env->FindClass("java/lang/Class");
        jobject classLoader = env->CallObjectMethod(
            env->FindClass("com/mojang/minecraftpe/MainActivity")
            , env->GetMethodID(classCls, "getClassLoader", "()Ljava/lang/ClassLoader;")
        );
        jclass dexClassLoaderCls = env->FindClass("dalvik/system/DexClassLoader");
        jobject dexClassLoader = env->NewObject(
            dexClassLoaderCls
            , env->GetMethodID(dexClassLoaderCls, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V")
            , env->NewStringUTF(MCPE_DATA_PATH)
            , env->NewStringUTF(MCPE_DATA_PATH_DUMMY)
            , NULL
            , classLoader
        );
        jclass n_playerCls = (jclass) env->CallStaticObjectMethod(
            classCls
            , env->GetStaticMethodID(classCls, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;")
            , env->NewStringUTF("com.rmpi.newmodpe.N_Player")
            , true
            , dexClassLoader
        );
        JNINativeMethod implementation[] = { { "nativeSetRoundAttack", "(ZZI)V", (void*) *nativeSetRoundAttack } };
        env->RegisterNatives(n_playerCls, implementation, 1);
        N_Player = (jclass) env->NewGlobalRef(n_playerCls);
        env->DeleteLocalRef(n_playerCls);
        jclass scriptStateCls = env->FindClass("net/zhuoweizhang/mcpelauncher/ScriptManager$ScriptState");
        ScriptState = (jclass) env->NewGlobalRef(scriptStateCls);
        env->DeleteLocalRef(scriptStateCls);
        jclass scriptableObjectCls = env->FindClass("org/mozilla/javascript/ScriptableObject");
        ScriptableObject = (jclass) env->NewGlobalRef(scriptableObjectCls);
        env->DeleteLocalRef(scriptableObjectCls);
        if (attachStatus == JNI_EDETACHED)
            bl_JavaVM->DetachCurrentThread();
    }

    hookSymbol("_ZN3MobC2ER21EntityDefinitionGroupRK26EntityDefinitionIdentifier", Mob$Mob_1);
    hookSymbol("_ZN3MobC2ER5Level", Mob$Mob_2);
    hookSymbol("_ZN3MobD2Ev", Mob$dMob);
    hookSymbol("_ZN15MinecraftClient14onPlayerLoadedER6Player", MinecraftClient$onPlayerLoaded);
    hookSymbol("_ZN15MinecraftClient6onTickEii", MinecraftClient$onTick);
	return JNI_VERSION_1_2;
}
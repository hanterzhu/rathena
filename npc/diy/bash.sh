#!/bin/bash

while true; do
    # 检查端口8211是否被占用
    if ! lsof -i:8211 > /dev/null; then
        echo "端口8211未被占用，服务器宕机"
        nohup /home/steam/.steam/SteamApps/common/PalServer/PalServer.sh>/home/steam/game.log &
    fi
    # 等待30秒
    sleep 30
done


#!/bin/bash

# 获取端口8211上的进程号
pid=$(lsof -t -i :8211)
port_pid=$(ps aux | grep check_port.sh | grep -v grep)

if [ -z "$port_pid" ]; then
    echo "没有发现守护进程。"
else
    kill $port_pid
    echo "守护进程已关闭。"
fi

# 检查是否成功获取到进程号
if [ -z "$pid" ]; then
    echo "没有发现监听在端口8211的进程。"
else
    # 杀死进程
    kill $pid
    echo "进程 $pid 已被终止。"
fi

# 重新启动服务器
echo "启动服务器。"
echo "等待15秒。"
sleep 15
nohup /home/steam/.steam/SteamApps/common/PalServer/PalServer.sh>/home/steam/game.log &
echo "服务器启动成功。"

echo "启动守护进程。"
echo "等待15秒。"
sleep 15
nohup /home/steam/check_port.sh>/home/steam/port.log &
echo "守护进程已启动。"

#!/bin/bash
pid=$(lsof -t -i :8211)
port_pid=$(ps aux | grep check_port.sh | grep -v grep)

if [ -z "$port_pid" ]; then
    echo "没有发现守护进程。"
else
    kill $port_pid
    echo "守护进程已关闭。"
fi

# 检查是否成功获取到进程号
if [ -z "$pid" ]; then
    echo "没有发现监听在端口8211的进程。"
else
    # 杀死进程
    kill $pid
    echo "进程 $pid 已被终止。"
fi


#!/bin/bash

# 设置要压缩的文件夹路径
FOLDER_TO_COMPRESS="/home/steam/.steam/SteamApps/common/PalServer/Pal/Saved/SaveGames"

# 获取当前日期和小时（格式如 YYYYMMDDHH）
CURRENT_DATE_HOUR=$(date +"%Y%m%d%H")

# 设置压缩文件的完整路径和文件名
DESTINATION_FOLDER="/home/steam/backup"
ARCHIVE_NAME="$DESTINATION_FOLDER/backup_$CURRENT_DATE_HOUR.tar.gz"

# 检查目标目录是否存在，如果不存在则创建
if [ ! -d "$DESTINATION_FOLDER" ]; then
    mkdir -p "$DESTINATION_FOLDER"
fi

# 压缩文件夹
tar -czf $ARCHIVE_NAME $FOLDER_TO_COMPRESS

echo "文件夹已压缩并保存到: $ARCHIVE_NAME"






/shutdown {秒} {messageText} 使用可选的计时器和/或消息正常关闭服务器，以通知服务器中的玩家。
/DoExit 立即强制关闭服务器。不建议使用此选项，除非您遇到技术问题或可以接受可能丢失数据的情况。
/Broadcast {MessageText} 向服务器中的所有玩家广播消息。
/KickPlayer {PlayerUID 或 SteamID} 将玩家踢出服务器。有助于适度地吸引玩家的注意力。
/BanPlayer {PlayerUID 或 SteamID} 禁止玩家进入服务器。玩家在解禁之前将无法重新加入服务器。
/TeleportToPlayer {PlayerUID 或 SteamID} 仅限游戏内
立即传送到目标玩家
/TeleportToMe {PlayerUID 或 SteamID} 仅限游戏内
立即将目标玩家传送到您身边。
/ShowPlayers（表演播放器） 显示所有已连接玩家的信息
/Info 显示服务器信息
/Save 将世界数据保存到磁盘。有助于确保您的好友、玩家和其他数据在停止服务器或执行有风险的游戏选项之前得到保存。

[/Script/Pal.PalGameWorldSettings]
OptionSettings=(Difficulty=None,DayTimeSpeedRate=1.000000,NightTimeSpeedRate=1.000000,ExpRate=1.000000,PalCaptureRate=1.000000,PalSpawnNumRate=1.000000,PalDamageRateAttack=1.000000,PalDamageRateDefense=1.000000,PlayerDamageRateAttack=1.000000,PlayerDamageRateDefense=1.000000,PlayerStomachDecreaceRate=1.000000,PlayerStaminaDecreaceRate=1.000000,PlayerAutoHPRegeneRate=1.000000,PlayerAutoHpRegeneRateInSleep=1.000000,PalStomachDecreaceRate=1.000000,PalStaminaDecreaceRate=1.000000,PalAutoHPRegeneRate=1.000000,PalAutoHpRegeneRateInSleep=1.000000,BuildObjectDamageRate=0.000000,BuildObjectDeteriorationDamageRate=0.000000,CollectionDropRate=1.000000,CollectionObjectHpRate=1.000000,CollectionObjectRespawnSpeedRate=1.000000,EnemyDropItemRate=1.000000,DeathPenalty=ItemAndEquipment,bEnablePlayerToPlayerDamage=False,bEnableFriendlyFire=True,bEnableInvaderEnemy=True,bActiveUNKO=False,bEnableAimAssistPad=True,bEnableAimAssistKeyboard=False,DropItemMaxNum=3000,DropItemMaxNum_UNKO=100,BaseCampMaxNum=128,BaseCampWorkerMaxNum=15,DropItemAliveMaxHours=1.000000,bAutoResetGuildNoOnlinePlayers=False,AutoResetGuildTimeNoOnlinePlayers=72.000000,GuildPlayerMaxNum=20,PalEggDefaultHatchingTime=0.000000,WorkSpeedRate=1.000000,bIsMultiplay=False,bIsPvP=False,bCanPickupOtherGuildDeathPenaltyDrop=False,bEnableNonLoginPenalty=True,bEnableFastTravel=True,bIsStartLocationSelectByMap=True,bExistPlayerAfterLogout=False,bEnableDefenseOtherGuildPlayer=False,CoopPlayerMaxNum=4,ServerPlayerMaxNum=12,ServerName="Default PalWorld Server",ServerDescription="",AdminPassword="19890129",ServerPassword="",PublicPort=8211,PublicIP="",RCONEnabled=False,RCONPort=25575,Region="",bUseAuth=True,BanListURL="https://api.palworldgame.com/api/banlist.txt")





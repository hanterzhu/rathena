#!/bin/bash

while true; do
    # ���˿�8211�Ƿ�ռ��
    if ! lsof -i:8211 > /dev/null; then
        echo "�˿�8211δ��ռ�ã�������崻�"
        nohup /home/steam/.steam/SteamApps/common/PalServer/PalServer.sh>/home/steam/game.log &
    fi
    # �ȴ�30��
    sleep 30
done


#!/bin/bash

# ��ȡ�˿�8211�ϵĽ��̺�
pid=$(lsof -t -i :8211)
port_pid=$(ps aux | grep check_port.sh | grep -v grep)

if [ -z "$port_pid" ]; then
    echo "û�з����ػ����̡�"
else
    kill $port_pid
    echo "�ػ������ѹرա�"
fi

# ����Ƿ�ɹ���ȡ�����̺�
if [ -z "$pid" ]; then
    echo "û�з��ּ����ڶ˿�8211�Ľ��̡�"
else
    # ɱ������
    kill $pid
    echo "���� $pid �ѱ���ֹ��"
fi

# ��������������
echo "������������"
echo "�ȴ�15�롣"
sleep 15
nohup /home/steam/.steam/SteamApps/common/PalServer/PalServer.sh>/home/steam/game.log &
echo "�����������ɹ���"

echo "�����ػ����̡�"
echo "�ȴ�15�롣"
sleep 15
nohup /home/steam/check_port.sh>/home/steam/port.log &
echo "�ػ�������������"

#!/bin/bash
pid=$(lsof -t -i :8211)
port_pid=$(ps aux | grep check_port.sh | grep -v grep)

if [ -z "$port_pid" ]; then
    echo "û�з����ػ����̡�"
else
    kill $port_pid
    echo "�ػ������ѹرա�"
fi

# ����Ƿ�ɹ���ȡ�����̺�
if [ -z "$pid" ]; then
    echo "û�з��ּ����ڶ˿�8211�Ľ��̡�"
else
    # ɱ������
    kill $pid
    echo "���� $pid �ѱ���ֹ��"
fi


#!/bin/bash

# ����Ҫѹ�����ļ���·��
FOLDER_TO_COMPRESS="/home/steam/.steam/SteamApps/common/PalServer/Pal/Saved/SaveGames"

# ��ȡ��ǰ���ں�Сʱ����ʽ�� YYYYMMDDHH��
CURRENT_DATE_HOUR=$(date +"%Y%m%d%H")

# ����ѹ���ļ�������·�����ļ���
DESTINATION_FOLDER="/home/steam/backup"
ARCHIVE_NAME="$DESTINATION_FOLDER/backup_$CURRENT_DATE_HOUR.tar.gz"

# ���Ŀ��Ŀ¼�Ƿ���ڣ�����������򴴽�
if [ ! -d "$DESTINATION_FOLDER" ]; then
    mkdir -p "$DESTINATION_FOLDER"
fi

# ѹ���ļ���
tar -czf $ARCHIVE_NAME $FOLDER_TO_COMPRESS

echo "�ļ�����ѹ�������浽: $ARCHIVE_NAME"






/shutdown {��} {messageText} ʹ�ÿ�ѡ�ļ�ʱ����/����Ϣ�����رշ���������֪ͨ�������е���ҡ�
/DoExit ����ǿ�ƹرշ�������������ʹ�ô�ѡ����������������������Խ��ܿ��ܶ�ʧ���ݵ������
/Broadcast {MessageText} ��������е�������ҹ㲥��Ϣ��
/KickPlayer {PlayerUID �� SteamID} ������߳����������������ʶȵ�������ҵ�ע������
/BanPlayer {PlayerUID �� SteamID} ��ֹ��ҽ��������������ڽ��֮ǰ���޷����¼����������
/TeleportToPlayer {PlayerUID �� SteamID} ������Ϸ��
�������͵�Ŀ�����
/TeleportToMe {PlayerUID �� SteamID} ������Ϸ��
������Ŀ����Ҵ��͵�����ߡ�
/ShowPlayers�����ݲ������� ��ʾ������������ҵ���Ϣ
/Info ��ʾ��������Ϣ
/Save ���������ݱ��浽���̡�������ȷ�����ĺ��ѡ���Һ�����������ֹͣ��������ִ���з��յ���Ϸѡ��֮ǰ�õ����档

[/Script/Pal.PalGameWorldSettings]
OptionSettings=(Difficulty=None,DayTimeSpeedRate=1.000000,NightTimeSpeedRate=1.000000,ExpRate=1.000000,PalCaptureRate=1.000000,PalSpawnNumRate=1.000000,PalDamageRateAttack=1.000000,PalDamageRateDefense=1.000000,PlayerDamageRateAttack=1.000000,PlayerDamageRateDefense=1.000000,PlayerStomachDecreaceRate=1.000000,PlayerStaminaDecreaceRate=1.000000,PlayerAutoHPRegeneRate=1.000000,PlayerAutoHpRegeneRateInSleep=1.000000,PalStomachDecreaceRate=1.000000,PalStaminaDecreaceRate=1.000000,PalAutoHPRegeneRate=1.000000,PalAutoHpRegeneRateInSleep=1.000000,BuildObjectDamageRate=0.000000,BuildObjectDeteriorationDamageRate=0.000000,CollectionDropRate=1.000000,CollectionObjectHpRate=1.000000,CollectionObjectRespawnSpeedRate=1.000000,EnemyDropItemRate=1.000000,DeathPenalty=ItemAndEquipment,bEnablePlayerToPlayerDamage=False,bEnableFriendlyFire=True,bEnableInvaderEnemy=True,bActiveUNKO=False,bEnableAimAssistPad=True,bEnableAimAssistKeyboard=False,DropItemMaxNum=3000,DropItemMaxNum_UNKO=100,BaseCampMaxNum=128,BaseCampWorkerMaxNum=15,DropItemAliveMaxHours=1.000000,bAutoResetGuildNoOnlinePlayers=False,AutoResetGuildTimeNoOnlinePlayers=72.000000,GuildPlayerMaxNum=20,PalEggDefaultHatchingTime=0.000000,WorkSpeedRate=1.000000,bIsMultiplay=False,bIsPvP=False,bCanPickupOtherGuildDeathPenaltyDrop=False,bEnableNonLoginPenalty=True,bEnableFastTravel=True,bIsStartLocationSelectByMap=True,bExistPlayerAfterLogout=False,bEnableDefenseOtherGuildPlayer=False,CoopPlayerMaxNum=4,ServerPlayerMaxNum=12,ServerName="Default PalWorld Server",ServerDescription="",AdminPassword="19890129",ServerPassword="",PublicPort=8211,PublicIP="",RCONEnabled=False,RCONPort=25575,Region="",bUseAuth=True,BanListURL="https://api.palworldgame.com/api/banlist.txt")





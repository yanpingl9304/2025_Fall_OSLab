# 編譯模組
make

# 插入模組
sudo insmod osfs.ko

# 檢查模組是否已插入
sudo dmesg

# 掛載檔案系統
sudo mount -t osfs none mnt/

# 檢查掛載是否成功
sudo dmesg

# 建立檔案
sudo touch mnt/test1.txt

# 寫入資料
sudo bash -c "echo 'I LOVE OSLAB' > mnt/test1.txt"

# 檢查檔案內容
cat mnt/test1.txt

# 卸載檔案系統
sudo umount mnt/

# 卸載模組
sudo rmmod osfs

make clean
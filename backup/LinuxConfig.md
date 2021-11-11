# 配置 Linux 环境

到手一台新的 Linux 机器，记录配置 Linux 全过程，以便之后再次配置，仅为自用。

注：发行版为 Debian 10



## 1. 用户相关

首先使用 root 账户登录，接下来就是用户相关操作，创建账号，修改密码，分配权限等操作。

```sh
# create user
user -m some_username   # -m 为创建 /home/some_username 目录
passwd some_username    # 为 some_username 用户修改密码
```

接下来就可以尝试使用 `some_username` 作为用户登录机器了，如果 `some_username` 需要 `sudo` 权限，可以试一下 `sudo` 命令是否能执行。如果执行失败可能需要配置 sudoers。

需要向 `/etc/sudoers` 中追加下面内容：

```txt
some_username ALL=(ALL) NOPASSWD:ALL
```



## 2. 主机名

有的机器会设置一串乱七八糟的主机名，想要看着舒服，可以修改主机名，需要在 root 权限下操作

```sh
echo some_hostname > /etc/hostname
```

之后打开 `/etc/hosts` 文件，找到本机旧的 `hostname`，改成 `some_hostname` 

之后重启机器即可



## 3. 配置 zsh

接下来使用 `some_username` 账户登录

注：第一次 `install` 之前需要执行 `sudo apt update`

```sh
sudo apt install zsh
sh -c "$(curl -fsSL https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"  # or
sh -c "$(wget https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh -O -)"
```



编辑 `~/.zshrc`，修改主题，个人比较喜欢 `ys`：

```sh
# ZSH_THEME="robbyrussell"
ZSH_THEME="ys"
```



## 4. 安装常用软件

下面是一些对人类友好的软件

```sh
# 快速搜索文件/文件内容，某些情况下代替 find, grep, ack 等
sudo apt install silversearcher-ag

# 取代 du 命令
sudo apt install ncdu

# 比 wget, curl 更快
sudo apt install axel

# 基本去掉 top
sudo apt install htop

# 方便在 Windows/Linux 传输文件
sudo apt install lrzsz
```



## 5. 编程语言环境

**C/C++**

以下软件可选择安装，也可以使用 `gcc/g++` 代替 `clang`

```sh
sudo apt install clang cmake gdb valgrind
```



**Java**

为了搭我的世界服务器，需要 Java 环境。目前(2021.11) Debian10 默认安装 jdk11，下面分别描述安装 jdk11 和 jdk8 过程

```sh
# install Java 11
sudo apt install default-jdk

# install Java 8
sudo apt install apt-transport-https ca-certificates wget dirmngr gnupg software-properties-common
wget -qO - https://adoptopenjdk.jfrog.io/adoptopenjdk/api/gpg/key/public | sudo apt-key add -
sudo add-apt-repository --yes https://adoptopenjdk.jfrog.io/adoptopenjdk/deb/
sudo apt update
sudo apt install adoptopenjdk-8-hotspot
```



**Python**

```sh
sudo apt install python3
```

可能机器已经安装 Python2 或 Python3，如果需要切换默认 Python 版本，比如希望默认为 Python3，可以使用以下两种方法：

```sh
# 1. 仅对一个用户生效
echo "alias python=`which python3`" >> ~/.zshrc

# 2. 修改 python 软链接，使其指向 Python3
sudo ln -s /usr/bin/python3.7 /usr/bin/python
```

安装 `pip3`

```sh
axel -n 20 https://bootstrap.pypa.io/get-pip.py
python3 get-pip.py
python3 -m pip --version  # 就可以看到安装成功了
```



## 6. 配置 vimrc

懒得自己搭配，我直接使用 https://github.com/amix/vimrc 配置

为单个用户配置 Awesome 版本：

```sh
git clone --depth=1 https://github.com/amix/vimrc.git ~/.vim_runtime
sh ~/.vim_runtime/install_awesome_vimrc.sh
```


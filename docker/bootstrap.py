import os
import shutil
import logging
from configparser import ConfigParser

logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')

# COOLQ_DIR = '/home/user/coolq'
COOLQ_DIR = '/home/richard/Lab/coolq'
COOLQ_CONFIG_FILE = os.path.join(COOLQ_DIR, 'conf', 'CQP.cfg')
COOLQ_APP_DIR = os.path.join(COOLQ_DIR, 'app')
APP_ID = 'io.github.richardchien.coolqhttpapi'
APP_DIR = os.path.join(COOLQ_APP_DIR, APP_ID)
APP_CONFIG_DIR = os.path.join(APP_DIR, 'config')
VERSION_LOCK_FILE = os.path.join(APP_DIR, 'version.lock')

# CPK_FILE = '/home/user/io.github.richardchien.coolqhttpapi.cpk'
CPK_FILE = '/home/richard/Lab/coolq/io.github.richardchien.coolqhttpapi.cpk'


def touch(path):
    with open(path, 'a'):
        os.utime(path, None)


def copy_cpk():
    shutil.copyfile(
        CPK_FILE,
        os.path.join(COOLQ_APP_DIR, APP_ID + '.cpk')
    )


def enable_plugin():
    coolq_config = ConfigParser()
    coolq_config.read(COOLQ_CONFIG_FILE)
    if 'App' in coolq_config:
        coolq_config['App'][APP_ID + '.status'] = '1'
    else:
        coolq_config['App'] = {
            APP_ID + '.status': '1'
        }
    with open(COOLQ_CONFIG_FILE, 'w') as config_file:
        coolq_config.write(config_file)


def is_first_start():
    return not os.path.exists(APP_DIR)


def version_locked():
    return os.path.exists(VERSION_LOCK_FILE)


def force_env():
    f = os.getenv('FORCE_ENV', 'false')
    return f.strip().lower() in ['1', 'yes', 'true']


def bootstrap():
    if is_first_start():
        logging.debug('容器首次启动，开始初始化……')
        os.makedirs(APP_DIR, exist_ok=True)
        logging.debug('正在安装插件……')
        copy_cpk()
        touch(VERSION_LOCK_FILE)
        logging.debug('正在启用插件……')
        enable_plugin()
    elif version_locked():
        logging.debug('插件版本已锁定，开始覆盖 cpk 文件……')
        copy_cpk()

    config_name = os.getenv('COOLQ_ACCOUNT', 'general')
    app_config = ConfigParser()
    app_config['general'] = {}
    for key, value in os.environ.items():
        if not key.startswith('CQHTTP_'):
            continue

        key = key[len('CQHTTP_'):]
        app_config[config_name][key.lower()] = value

    def write_config():
        os.makedirs(APP_CONFIG_DIR, mode=0o755, exist_ok=True)
        with open(os.path.join(APP_CONFIG_DIR, config_name + '.ini'), 'w') as config_file:
            app_config.write(config_file)

    if not os.path.exists(APP_CONFIG_DIR):
        logging.debug('配置文件不存在，正在根据配置文件生成初始配置文件……')
        write_config()
    elif force_env():
        logging.debug('已强制使用环境变量，正在覆盖配置文件……')
        shutil.rmtree(APP_CONFIG_DIR, ignore_errors=True)
        write_config()

    logging.info('CoolQ HTTP API 插件 bootstrap 完成。')


if __name__ == '__main__':
    bootstrap()

import os
import json
import shutil
import logging
from configparser import ConfigParser

logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')

APP_ID = 'io.github.richardchien.coolqhttpapi'

COOLQ_DIR = '/home/user/coolq'
COOLQ_CONFIG_FILE = os.path.join(COOLQ_DIR, 'conf', 'CQP.cfg')
COOLQ_APP_DIR = os.path.join(COOLQ_DIR, 'app')
COOLQ_APP_DATA_DIR = os.path.join(COOLQ_DIR, 'data', 'app')

APP_DIR_OLD = os.path.join(COOLQ_APP_DIR, APP_ID)
APP_DIR = os.path.join(COOLQ_APP_DATA_DIR, APP_ID)
APP_CONFIG_DIR = os.path.join(APP_DIR, 'config')
VERSION_LOCK_FILE = os.path.join(APP_DIR, 'version.lock')

CPK_NAME = f'{APP_ID}.cpk'
CPK_FILE = f'/home/user/{CPK_NAME}'


def strtobool(s):
    return s.lower() in {'1', 'true', 'yes', 'on'}


def touch(path):
    with open(path, 'a'):
        os.utime(path, None)


def makedirs(dir, mode=0o755, exist_ok=True):
    os.makedirs(dir, mode=mode, exist_ok=exist_ok)


def copy_cpk():
    makedirs(COOLQ_APP_DIR)
    shutil.copyfile(CPK_FILE, os.path.join(COOLQ_APP_DIR, CPK_NAME))


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
    try:
        return strtobool(os.getenv('FORCE_ENV', 'false'))
    except ValueError:
        return False


def app_config_paths(config_name, include_general=False):
    paths = [
        os.path.join(APP_CONFIG_DIR, config_name + '.ini'),
        os.path.join(APP_CONFIG_DIR, config_name + '.cfg'),
        os.path.join(APP_CONFIG_DIR, config_name + '.json'),
    ]
    if include_general:
        paths.extend([
            os.path.join(APP_CONFIG_DIR, 'general.ini'),
            os.path.join(APP_CONFIG_DIR, 'general.cfg'),
            os.path.join(APP_CONFIG_DIR, 'general.json'),
        ])
    return paths


def app_config_exists(config_name, include_general=False):
    for p in app_config_paths(config_name, include_general):
        if os.path.exists(p):
            return True
    return False


def remove_app_config(config_name, include_general=False):
    for p in app_config_paths(config_name, include_general):
        if os.path.exists(p):
            os.remove(p)


def app_config_format():
    if os.path.exists(os.path.join(APP_CONFIG_DIR, 'general.json')) and \
            not os.path.exists(
                os.path.join(APP_CONFIG_DIR, 'general.ini')) and \
            not os.path.exists(
                os.path.join(APP_CONFIG_DIR, 'general.cfg')):
        return 'json'
    return 'ini'


def write_app_config(config_name, app_config):
    makedirs(APP_CONFIG_DIR)
    file_format = app_config_format()
    with open(os.path.join(APP_CONFIG_DIR, config_name + '.' + file_format),
              'w') as config_file:
        if file_format == 'json':
            json.dump(dict(app_config[config_name].items()),
                      config_file, ensure_ascii=False, indent=4)
        else:
            app_config.write(config_file)


def move_old_app_dir():
    if os.path.exists(APP_DIR_OLD) and not os.path.exists(APP_DIR):
        logging.debug(f'检测到旧的插件数据，正在复制到新位置 {APP_DIR}……')
        makedirs(os.path.dirname(APP_DIR))
        shutil.copytree(APP_DIR_OLD, APP_DIR)
        app_data_backup_dir = f'{APP_DIR_OLD.rstrip(os.path.sep)}.bak'
        shutil.move(APP_DIR_OLD, app_data_backup_dir)
        logging.debug(f'复制插件数据成功，旧的插件数据已备份在 {app_data_backup_dir}。')


def bootstrap():
    move_old_app_dir()

    if is_first_start():
        logging.debug('容器首次启动，开始初始化……')
        makedirs(APP_DIR)
        logging.debug('正在安装插件……')
        copy_cpk()
        touch(VERSION_LOCK_FILE)
        logging.debug('正在启用插件……')
        enable_plugin()
    elif version_locked():
        logging.debug('插件版本已锁定，正在覆盖 cpk 文件……')
        copy_cpk()

    config_name = os.getenv('COOLQ_ACCOUNT', 'general')
    app_config = ConfigParser()
    app_config[config_name] = {}
    for key, value in os.environ.items():
        if not key.startswith('CQHTTP_'):
            continue

        key = key[len('CQHTTP_'):]
        app_config[config_name][key.lower()] = value

    if not app_config_exists(config_name, include_general=False):
        logging.debug('配置文件不存在，正在根据配置文件生成初始配置文件……')
        write_app_config(config_name, app_config)
    elif force_env():
        logging.debug('已强制使用环境变量，正在覆盖配置文件……')
        remove_app_config(config_name, include_general=False)
        write_app_config(config_name, app_config)

    logging.info('CQHTTP 插件 bootstrap 成功。')


if __name__ == '__main__':
    try:
        bootstrap()
    except Exception as e:
        logging.exception(e)
        logging.error('CQHTTP 插件 bootstrap 失败。')

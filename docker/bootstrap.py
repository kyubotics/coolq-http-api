import os
import shutil
from configparser import ConfigParser

COOLQ_DIR = '/home/user/coolq'
COOLQ_CONFIG_FILE = os.path.join(COOLQ_DIR, 'conf', 'CQP.cfg')
COOLQ_APP_DIR = os.path.join(COOLQ_DIR, 'app')
APP_ID = 'io.github.richardchien.coolqhttpapi'
APP_DIR = os.path.join(COOLQ_APP_DIR, APP_ID)
APP_CONFIG_DIR = os.path.join(APP_DIR, 'config')
VERSION_LOCK_FILE = os.path.join(APP_DIR, 'version.lock')

CPK_FILE = '/home/user/io.github.richardchien.coolqhttpapi.cpk'


def touch(path):
    with open(path, 'a'):
        os.utime(path, None)


def copy_cpk():
    # copy cpk
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


def init():
    os.makedirs(APP_DIR, exist_ok=True)
    os.makedirs(APP_CONFIG_DIR, exist_ok=True)
    copy_cpk()
    touch(VERSION_LOCK_FILE)
    enable_plugin()


def version_locked():
    return os.path.exists(VERSION_LOCK_FILE)


def config_manually():
    use = os.getenv('CONFIG_MANUALLY', 'false')
    return use.strip().lower() in ['1', 'yes', 'true']


def bootstrap():
    if is_first_start():
        init()

    if version_locked():
        # override cpk file
        copy_cpk()

    if not config_manually():
        # override app config
        config_name = os.getenv('COOLQ_ACCOUNT', 'general')
        app_config = ConfigParser()
        app_config['general'] = {}
        for key, value in os.environ.items():
            if not key.startswith('CQHTTP_'):
                continue

            key = key[len('CQHTTP_'):]
            app_config['general'][key.lower()] = value
        with open(os.path.join(APP_CONFIG_DIR, config_name + '.ini'), 'w') as config_file:
            app_config.write(config_file)


if __name__ == '__main__':
    bootstrap()

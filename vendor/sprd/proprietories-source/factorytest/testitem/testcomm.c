#include "testitem.h"

struct sensors_poll_device_t *device;
struct sensors_module_t *module;
struct sensor_t const *list;
int count = 0;
int senloaded = -1;

#define S_ON    1
#define S_OFF   0
extern const char INPUT_EVT_NAME[];

int get_sensor_name(const char * name )
{
    int fd = -1;
    int i = 0;
    char devName[64] = { 0 };
    char EvtName[64] = { 0 };
    struct stat stt;

    int EvtNameLen = strlen(INPUT_EVT_NAME);
    strcpy(EvtName, INPUT_EVT_NAME);
    EvtName[EvtNameLen + 1] = 0;

    for(i = 0; i < 16; ++i ) {
        EvtName[EvtNameLen] = (char)('0' + i);
        LOGD("input evt name = %s", EvtName);

        if( stat(EvtName, &stt) != 0 ) {
            LOGE("stat '%s' error!",EvtName);
            break;
        }

        fd = open(EvtName, O_RDONLY);
        if( fd < 0 ) {
            LOGE("Failed to open %s", EvtName);
            continue;
        }

        if( ioctl(fd, EVIOCGNAME(sizeof(devName)), devName) > 0 &&  strstr(devName, name) != NULL ) {
            LOGD("open '%s' OK, dev fd = %d", devName, fd);
            break;
        }

        LOGD("input evt name = %s, dev name = %s", EvtName, devName);
        close(fd);
        fd = -1;
    }

    if( fd >= 0 ) {
        if( fcntl(fd, F_SETFL, O_NONBLOCK) < 0 ) {
            LOGE("fcntl: set to nonblock error!");
        }
    }

    return fd;
}

int getSensorId(const char *sensorname)
{
	int i;

	for(i = 0 ; i < count; i++)
	{
		LOGD("get sensorname = %s, ID = %d, sensor name = %s!!", sensorname, i, list[i].name);
		if(strstr(list[i].name, sensorname))
		{
			LOGD("get sensor success! ID = %d, sensor name = %s, type = %d!!", i, list[i].name, list[i].type);
			return i;
		}
	}
	LOGD("failed to get the sensor ID!!");
	return -1;
}

int find_input_dev(int mode, const char *event_name)
{
	int fd = -1;
	int ret = -1;
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;
	char name[128];

	dir = opendir(dirname);
	if (dir == NULL) {
		return -1;
	}

	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';

	while ((de = readdir(dir))) {
		if ((de->d_name[0] == '.' && de->d_name[1] == '\0') ||
				(de->d_name[0] == '.' && de->d_name[1] == '.'  &&
				 de->d_name[2] == '\0')) {
			/* ignore .(current) and ..(top) directory */
			continue;
		}
		strcpy(filename, de->d_name);
		fd = open(devname, mode);

		if (fd >= 0) {
			memset(name, 0, sizeof(name));
			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 0) {

			} else {
				if (!strcmp(name, event_name)) {
					ret = fd; //get the sensor name from the event
					goto END;
				}
			}
			close(fd);
		}
	}
END:
	closedir(dir);

	return ret;
}

static int activate_sensors(int id, int delay, int opt)
{
#ifndef HAL_VERSION_1_3
    int err;
    err = device->activate(device, list[id].handle, 0);
    if (err != 0) {
        return 0;
    }
    if (!opt) {
        return 0;
    }
    err = device->activate(device, list[id].handle, 1);
    if (err != 0) {
        return 0;
    }
    device->setDelay(device, list[id].handle, ms2ns(delay));
    return err;

#else

	int err;
	struct sensors_poll_device_1 * dev = (struct sensors_poll_device_1 *)device;

	LOGD("sensor parameters: dalay = %d; opt = %d! %d IN",delay, opt, __LINE__);
	err = device->activate(device, list[id].handle, 0);
	if (err != 0) {
		return -1;
	}
	if (!opt) {
		return -1;
	}
	LOGD("active sensor 0! %d IN", __LINE__);
	err = device->activate(device, list[id].handle, 1);
	LOGD("device->activate: err = %d! %d IN", err, __LINE__);
	if (err != 0) {
		return -1;
	}
	LOGD("active sensor 1! %d IN", __LINE__);
	dev->flush(dev, list[id].handle);
	LOGD("flush sensor event! %d IN", __LINE__);
	dev->batch(dev, list[id].handle, 1, ms2ns(delay), 0);
	SPRD_DBG("Sets a sensor's parameters! %d IN", __LINE__);

	return err;

#endif
}

int enable_sensor()
{
    int i,err;

    for ( i = 0; i < count; i++) {
        err = activate_sensors(i, 1, S_ON);
        LOGD("activate_sensors(ON) for '%s'", list[i].name);
        if (err != 0) {
            LOGE("activate_sensors(ON) for '%s'failed", list[i].name);
            return 0;
        }
    }

    return err;
}

int sensor_enable(const char *sensorname)
{
	int id = -1;
	int rtn = -1;

	id = getSensorId(sensorname);
	rtn = activate_sensors(id, 50, S_ON);
	LOGD("activate_sensors(ON) for '%s'", list[id].name);
	if (rtn != 0)
	{
		LOGE("activate_sensors(ON) for '%s'failed", list[id].name);
		return -1;
	}

	return id;
}

int sensor_disable(const char *sensorname)
{
	int i,err;

	/*********activate_sensors(OFF)***************/
	i = getSensorId(sensorname);
	err = activate_sensors(i, 0, S_OFF);
	if (err != 0) {
		LOGE("activate_sensors(OFF) for '%s'failed", list[i].name);
		return -1;
	}
#if 0
	/*********close sensor***************/
	err = sensors_close(device);
	if (err != 0) {
		LOGE("mmitest sensors_close() failed");
	}
#endif
	return i;
}

int sensor_stop()
{
    int i,err;

    /*********activate_sensors(OFF)***************/
    for (i = 0; i < count; i++) {
        err = activate_sensors(i, 0, S_OFF);
        if (err != 0) {
            LOGE("activate_sensors(OFF) for '%s'failed", list[i].name);
            return 0;
        }
    }
    /*********close sensor***************/
    /*err = sensors_close(device);
    if (err != 0) {
        LOGE("mmitest sensors_close() failed");
    }*/
    return err;
}

int sensor_load()
{
    static int is_sensor_load = 0;
    int err;
    if (is_sensor_load != 0){
        return 0;
    }
	/*********load sensor.so***************/
	err = hw_get_module(SENSORS_HARDWARE_MODULE_ID,
			    (hw_module_t const **)&module);
	if (err != 0) {
		LOGD("mmitest hw_get_module() failed (%s)", strerror(-err));
		return -1;
	}
	LOGD("get module success!!");
	/*********open sensor***************/
	err = sensors_open(&module->common, &device);
	if (err != 0) {
		LOGD("mmitest sensors_open() failed (%s)", strerror(-err));
		return -1;
	}
    is_sensor_load = 1;
	LOGD("sensor open success");
	/*********read cmd***************/
	count = module->get_sensors_list(module, &list);
	LOGD("sensor list get success!!count = %d!", count);
	return err;
}

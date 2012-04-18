#include "libubat.h"


/**
 * get_battery(char *battery)
 * 
 * Get the battery's pointer.
 * @param battery - A gchar array containing a string such as "BAT0"
 * @return A structure containing the battery's information.
 **/
struct BatteryInfo *init_battery(char *battery)
{
	char battery_path[64];
	if (strlen(battery) > 4)
	{
		fprintf(stderr, "Error: too many characters for battery.\n");
		return NULL;
	}

	struct BatteryInfo *info = malloc(sizeof(struct BatteryInfo));
	if (info == NULL) {
		fprintf(stderr, "Error: unable to initiate the battery");
		return NULL;
	}


	sprintf(battery_path, "/sys/class/power_supply/%s", battery);		
	FILE *data = NULL;
	if ((data = fopen(battery_path, "r")) == NULL) {	
		fprintf(stderr, "Error: no data for battery. (Is %s in /sys populated?)\n", battery);
		return NULL;
	}
	fclose(data);

	

	sprintf(battery_path, "/sys/class/power_supply/%s/energy_full", battery);	
	if ((data = fopen(battery_path, "r")) != NULL) {
		fscanf(data, "%d", &info->max);
		info->features |= 4;
		fclose(data);	
	}		
	else {
		sprintf(battery_path, "/sys/class/power_supply/%s/voltage_max", battery);	
		if ((data = fopen(battery_path, "r")) != NULL) {
			info->features |= 8;
			fscanf(data, "%d", &info->max);	
			fclose(data);
			sprintf(battery_path, "/sys/class/power_supply/%s/voltage_min", battery);
			if ((data = fopen(battery_path, "r")) != NULL) {
				fscanf(data, "%d", &info->min);
				fclose(data);
			}
			else {
				info->min = 0;
			}
		}
		else {
			sprintf(battery_path, "/sys/class/power_supply/%s/charge_full", battery);	
			if ((data = fopen(battery_path, "r")) != NULL) {
				fscanf(data, "%d", &info->max);	
				fclose(data);
			}
		}	
	 
	}	
	
	strcpy(info->battery, battery);
	update_battery(info);
	return info;	
}


/**
 * update_battery(struct BatteryInfo *info)
 * 
 * Update the battery info's capacity, status, and so on.
 * @param info - The battery info (already initiated by get_battery)
 **/
int update_battery(struct BatteryInfo *info)
{
	FILE *data = NULL;
	char battery_path[64];

	if (info->features & 4) 
		sprintf(battery_path, "/sys/class/power_supply/%s/energy_now", info->battery);	
	else if (info->features & 8)
		sprintf(battery_path, "/sys/class/power_supply/%s/voltage_now", info->battery);	
	else
		sprintf(battery_path, "/sys/class/power_supply/%s/charge_now", info->battery);	

	
	/* load the battery capacity */
	if ((data = fopen(battery_path, "r")) != NULL) {
		fscanf(data, "%d", &info->capacity);
		fclose(data);	
	}		
	
	/* load the battery's state */
	char c = 'k';
	
	sprintf(battery_path, "/sys/class/power_supply/%s/status", info->battery);	

	if ((data = fopen(battery_path, "r")) != NULL) {
		fscanf(data, "%c", &c);
		fclose(data);	
	}
	
	if (c == 'k') 
		info->status = MISSING;
	else if (c == 'D') 
		info->status = DISCHARGING;
	else if (c == 'C') {
		if (info->capacity == info->max) 
			info->status = CHARGED;
		else 
			info->status = CHARGING;
	}
	if (info->features & 8)
		info->percent = (int)((((float)info->capacity - (float)info->min) / ((float)info->max - (float)info->min)) * 100);
	else 
		info->percent = (int)((float)info->capacity / (float)info->max * 100.0);
}


/**
 * int get_rate(struct BatteryInfo *info)
 * 
 * Update the battery info's capacity, status, and so on.
 * @param info - The battery info (already initiated by get_battery)
 **/


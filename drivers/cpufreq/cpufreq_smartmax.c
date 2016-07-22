 attribute *b,
		const char *buf, size_t count) {
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0) {
		boost_freq = input;
		if (boost_freq == 0)
			boost = false;
		else
			boost = true;
	} else
		return -EINVAL;	
	return count;
}

static ssize_t show_boost_duration(struct kobject *kobj, struct attribute *attr,
		char *buf) {
	return sprintf(buf, "%d\n", boost_running);
}

static ssize_t store_boost_duration(struct kobject *a, struct attribute *b,
		const char *buf, size_t count) {
	ssize_t res;
	unsigned long input;
	res = strict_strtoul(buf, 0, &input);
	if (res >= 0 && input > 10000){
		boost_duration = input;
		if (boost) {
			// no need to bother if currently a boost is running anyway
			if (boost_task_alive && boost_running)
				return count;

			if (boost_task_alive) {
				cur_boost_freq = boost_freq;
				cur_boost_duration = boost_duration;
				wake_up_process(boost_task);
			}
		}
	} else
		return -EINVAL;
	return count;
}

static ssize_t show_io_is_busy(struct kobject *kobj, struct attribute *attr,
		char *buf) {
	return sprintf(buf, "%d\n", io_is_busy);
}

static ssize_t store_io_is_busy(struct kobject *a, struct attribute *b,
		const char *buf, size_t count) {
	ssize_t res;
	unsigned long input;

	res = strict_strtoul(buf, 0, &input);
	if (res >= 0) {
		if (input > 1)
			input = 1;
		if (input == io_is_busy) { /* nothing to do */
			return count;
		}
		io_is_busy = input;
	} else
		return -EINVAL;	
	return count;
}

static ssize_t show_ignore_nice(struct kobject *kobj, struct attribute *attr,
		char *buf) {
	return sprintf(buf, "%d\n", ignore_nice);
}

static ssize_t store_ignore_nice(struct kobject *a, struct attribute *b,
		const char *buf, size_t count) {
	ssize_t res;
	unsigned long input;

	res = strict_strtoul(buf, 0, &input);
	if (res >= 0) {
		if (input > 1)
			input = 1;
		if (input == ignore_nice) { /* nothing to do */
			return count;
		}
		ignore_nice = input;
		/* we need to re-evaluate prev_cpu_idle */
		update_idle_time(true);
	} else
		return -EINVAL;	
	return count;
}

#define define_global_rw_attr(_name)		\
static struct global_attr _name##_attr =	\
	__ATTR(_name, 0644, show_##_name, store_##_name)

define_global_rw_attr(debug_mask);
define_global_rw_attr(up_rate);
define_global_rw_attr(down_rate);
define_global_rw_attr(ramp_up_step);
define_global_rw_attr(ramp_down_step);
define_global_rw_attr(max_cpu_load);
define_global_rw_attr(min_cpu_load);
define_global_rw_attr(sampling_rate);
define_global_rw_attr(touch_poke_freq);
define_global_rw_attr(input_boost_duration);
define_global_rw_attr(boost_freq);
define_global_rw_attr(boost_duration);
define_global_rw_attr(io_is_busy);
define_global_rw_attr(ignore_nice);
define_global_rw_attr(ramp_up_during_boost);
define_global_rw_attr(awake_ideal_freq);
define_global_rw_attr(suspend_ideal_freq);

static struct attribute * smartmax_attributes[] = { 
	&debug_mask_attr.attr,
	&up_rate_attr.attr, 
	&down_rate_attr.attr, 
	&ramp_up_step_attr.attr, 
	&ramp_down_step_attr.attr,
	&max_cpu_load_attr.attr, 
	&min_cpu_load_attr.attr,
	&sampling_rate_attr.attr, 
	&touch_poke_freq_attr.attr,
	&input_boost_duration_attr.attr, 
	&boost_freq_attr.attr, 
	&boost_duration_attr.attr, 
	&io_is_busy_attr.attr,
	&ignore_nice_attr.attr, 
	&ramp_up_during_boost_attr.attr, 
	&awake_ideal_freq_attr.attr,
	&suspend_ideal_freq_attr.attr,		
	NULL , };

static struct attribute_group smartmax_attr_group = { .attrs =
		smartmax_attributes, .name = GOVERNOR_NAME, };

static int cpufreq_smartmax_boost_task(void *data) {
	struct cpufreq_policy *policy;
	struct smartmax_info_s *this_smartmax;
	cputime64_t now;

	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();

		if (kthread_should_stop())
			break;

		set_current_state(TASK_RUNNING);

		if (boost_running)
			continue;
		
		/* we always boost cpu 0 */
		this_smartmax = &per_cpu(smartmax_info, 0);
		if (!this_smartmax)
			continue;

		policy = this_smartmax->cur_policy;
		if (!policy)
			continue;

//		if (lock_policy_rwsem_write(0) < 0)
//			continue;

		mutex_lock(&this_smartmax->timer_mutex);

		if (policy->cur < cur_boost_freq) {
			boost_running = true;
		
			now = ktime_to_ns(ktime_get());
			boost_end_time = now + cur_boost_duration;
			dprintk(SMARTMAX_DEBUG_BOOST, "%s %llu %llu\n", __func__, now, boost_end_time);

			target_freq(policy, this_smartmax, cur_boost_freq, this_smartmax->old_freq, CPUFREQ_RELATION_H);
			this_smartmax->prev_cpu_idle = get_cpu_idle_time(0, &this_smartmax->prev_cpu_wall, io_is_busy);
		}
		mutex_unlock(&this_smartmax->timer_mutex);
				
//		unlock_policy_rwsem_write(0);
	}

	return 0;
}

static void dbs_input_event(struct input_handle *handle, unsigned int type,
		unsigned int code, int value) {
	if (touch_poke && type == EV_SYN && code == SYN_REPORT) {
		// no need to bother if currently a boost is running anyway
		if (boost_task_alive && boost_running)
			return;

		if (boost_task_alive) {
			cur_boost_freq = touch_poke_freq;
			cur_boost_duration = input_boost_duration;
			wake_up_process(boost_task);
		}
	}
}

static int input_dev_filter(const char* input_dev_name) {
	int ret = 0;
	if (strstr(input_dev_name, "touchscreen")
			|| strstr(input_dev_name, "-keypad")
			|| strstr(input_dev_name, "-nav")
			|| strstr(input_dev_name, "-oj")) {
	} else {
		ret = 1;
	}
	return ret;
}

static int dbs_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id) {
	struct input_handle *handle;
	int error;

	/* filter out those input_dev that we don't care */
	if (input_dev_filter(dev->name))
		return 0;

	dprintk(SMARTMAX_DEBUG_INPUT, "%s\n", __func__);

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpufreq";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;

	return 0;
	err1: input_unregister_handle(handle);
	err2: kfree(handle);
	return error;
}

static void dbs_input_disconnect(struct input_handle *handle) {
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id dbs_ids[] = { { .driver_info = 1 }, { }, };

static struct input_handler dbs_input_handler = { .event = dbs_input_event,
		.connect = dbs_input_connect, .disconnect = dbs_input_disconnect,
		.name = CPUFR_NAME, .id_table = dbs_ids, };

#ifdef CONFIG_POWERSUSPEND
static void smartmax_power_suspend(struct power_suspend *h)
{
	dprintk(SMARTMAX_DEBUG_SUSPEND, "%s\n", __func__);
	ideal_freq = suspend_ideal_freq;
	is_suspended = true;
	smartmax_update_min_max_allcpus();
}

static void smartmax_power_resume(struct power_suspend *h)
{
	dprintk(SMARTMAX_DEBUG_SUSPEND, "%s\n", __func__);
	ideal_freq = awake_ideal_freq;
	is_suspended = false;
	smartmax_update_min_max_allcpus();
}
#endif

static int FUNC_NAME(struct cpufreq_policy *new_policy,
		unsigned int event) {
	unsigned int cpu = new_policy->cpu;
	int rc;
	struct smartmax_info_s *this_smartmax = &per_cpu(smartmax_info, cpu);
	struct sched_param param = { .sched_priority = 1 };
    unsigned int latency;
    unsigned int min_sampling_rate;

	switch (event) {
	case CPUFREQ_GOV_START:
		if ((!cpu_online(cpu)) || (!new_policy->cur))return -EINVAL;

		mutex_lock(&dbs_mutex);

		this_smartmax->cur_policy = new_policy;
		this_smartmax->cpu = cpu;
		this_smartmax->enable = true;

		smartmax_update_min_max(this_smartmax,new_policy);

		this_smartmax->freq_table = cpufreq_frequency_get_table(cpu);

		update_idle_time(false);

		dbs_enable++;
		
		if (dbs_enable == 1) {
			if (!boost_task_alive) {
				boost_task = kthread_create (
						cpufreq_smartmax_boost_task,
						NULL,
						"kinputboostd"
				);

				if (IS_ERR(boost_task)) {
					dbs_enable--;
					mutex_unlock(&dbs_mutex);
					return PTR_ERR(boost_task);
				}

				sched_setscheduler_nocheck(boost_task, SCHED_RR, &param);
				get_task_struct(boost_task);
				boost_task_alive = true;
			}
			rc = input_register_handler(&dbs_input_handler);
			if (rc) {
				dbs_enable--;
				mutex_unlock(&dbs_mutex);
				return rc;
			}
			rc = sysfs_create_group(cpufreq_global_kobject,
					&smartmax_attr_group);
			if (rc) {
				dbs_enable--;
				mutex_unlock(&dbs_mutex);
				return rc;
			}
#ifdef CONFIG_POWERSUSPEND
			register_power_suspend(&smartmax_power_suspend_handler);
#endif
			latency = new_policy->cpuinfo.transition_latency / 1000;
			if (latency == 0)
				latency = 1;
			
			min_sampling_rate = max(sampling_rate, MIN_LATENCY_MULTIPLIER * latency);
			sampling_rate = max(min_sampling_rate, latency * LATENCY_MULTIPLIER);
		}

		mutex_unlock(&dbs_mutex);
		mutex_init(&this_smartmax->timer_mutex);
		dbs_timer_init(this_smartmax);

		break;
	case CPUFREQ_GOV_LIMITS:
		mutex_lock(&this_smartmax->timer_mutex);
		smartmax_update_min_max(this_smartmax,new_policy);

		if (this_smartmax->cur_policy->cur > new_policy->max) {
			dprintk(SMARTMAX_DEBUG_JUMPS,"jumping to new max freq: %d\n",new_policy->max);
			__cpufreq_driver_target(this_smartmax->cur_policy,
					new_policy->max, CPUFREQ_RELATION_H);
		}
		else if (this_smartmax->cur_policy->cur < new_policy->min) {
			dprintk(SMARTMAX_DEBUG_JUMPS,"jumping to new min freq: %d\n",new_policy->min);
			__cpufreq_driver_target(this_smartmax->cur_policy,
					new_policy->min, CPUFREQ_RELATION_L);
		}
		mutex_unlock(&this_smartmax->timer_mutex);
		break;

	case CPUFREQ_GOV_STOP:
		dbs_timer_exit(this_smartmax);

		mutex_lock(&dbs_mutex);
		mutex_destroy(&this_smartmax->timer_mutex);
		this_smartmax->enable = false;
		dbs_enable--;

		if (!dbs_enable){
			sysfs_remove_group(cpufreq_global_kobject, &smartmax_attr_group);
			input_unregister_handler(&dbs_input_handler);
#ifdef CONFIG_POWERSUSPEND
			unregister_power_suspend(&smartmax_power_suspend_handler);
#endif
		}
		
		mutex_unlock(&dbs_mutex);
		break;
	}

	return 0;
}

static int __init cpufreq_smartmax_init(void) {
	unsigned int i;
	struct smartmax_info_s *this_smartmax;
	up_rate = DEFAULT_UP_RATE;
	down_rate = DEFAULT_DOWN_RATE;
	suspend_ideal_freq = DEFAULT_SUSPEND_IDEAL_FREQ;
	awake_ideal_freq = DEFAULT_AWAKE_IDEAL_FREQ;
	ideal_freq = awake_ideal_freq;
	ramp_up_step = DEFAULT_RAMP_UP_STEP;
	ramp_down_step = DEFAULT_RAMP_DOWN_STEP;
	max_cpu_load = DEFAULT_MAX_CPU_LOAD;
	min_cpu_load = DEFAULT_MIN_CPU_LOAD;
	sampling_rate = DEFAULT_SAMPLING_RATE;
	input_boost_duration = DEFAULT_INPUT_BOOST_DURATION;
	io_is_busy = DEFAULT_IO_IS_BUSY;
	ignore_nice = DEFAULT_IGNORE_NICE;

	/* Initalize per-cpu data: */for_each_possible_cpu(i)
	{
		this_smartmax = &per_cpu(smartmax_info, i);
		this_smartmax->enable = false;
		this_smartmax->cur_policy = 0;
		this_smartmax->ramp_dir = 0;
		this_smartmax->freq_change_time = 0;
		this_smartmax->cur_cpu_load = 0;
	}

#ifdef CONFIG_POWERSUSPEND
	smartmax_power_suspend_handler.suspend = smartmax_power_suspend;
	smartmax_power_suspend_handler.resume = smartmax_power_resume;
#endif
	
	return cpufreq_register_governor(&STRUCT_NAME);
}

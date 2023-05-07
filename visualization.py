import re
import matplotlib.pyplot as plt
import plotly.figure_factory as ff
import colorlover as cl

with open("results.txt", "r") as file:
    data = file.read()

lines = data.split("\n")

num_jobs_list = []
num_machines_list = []
avg_run_time_list = []
avg_initial_make_span_list = []
avg_best_make_span_list = []
schedules_list = []

for line in lines:
    if not line.strip():
        continue

    num_jobs = int(re.search(r"NUM_JOBS:(\d+)", line).group(1))
    num_machines = int(re.search(r"NUM_MACHINES:(\d+)", line).group(1))
    avg_run_time = int(re.search(r"AVG_RUN_TIME:(\d+)", line).group(1))
    avg_initial_make_span = int(re.search(r"AVG_INITIAL_MAKE_SPAN:(\d+)", line).group(1))
    avg_best_make_span = int(re.search(r"AVG_BEST_MAKE_SPAN:(\d+)", line).group(1))

    matched_groups = re.search(r"SCHEDULE:\[(.*?)\]", line).group(1)
    print("Matched groups:", matched_groups)
    split_values = matched_groups.split(",")
    print("Split values:", split_values)

    filtered_values = list(filter(lambda x: x != "", split_values))
    print("Filtered values:", filtered_values)

    schedule = list(map(int, filtered_values))

    num_jobs_list.append(num_jobs)
    num_machines_list.append(num_machines)
    avg_run_time_list.append(avg_run_time)
    avg_initial_make_span_list.append(avg_initial_make_span)
    avg_best_make_span_list.append(avg_best_make_span)
    schedules_list.append(schedule)

plt.figure(figsize=(10, 6))
plt.plot(num_jobs_list, avg_run_time_list, label="Average Run Time")
plt.plot(num_jobs_list, avg_initial_make_span_list, label="Average Initial Make Span")
plt.plot(num_jobs_list, avg_best_make_span_list, label="Average Best Make Span")
plt.xlabel("Number of Jobs")
plt.ylabel("Time")
plt.legend()
plt.show()

tasks = []
for i, schedule in enumerate(schedules_list):
    for j, task_id in enumerate(schedule):
        task = f"Machine {task_id}"
        start = j * num_jobs_list[i]
        finish = start + num_jobs_list[i]
        tasks.append(dict(Task=f"Job {i}", Start=start, Finish=finish, Resource=task))

resource_set = set(task["Resource"] for task in tasks)
num_unique_resources = len(resource_set)
base_colorscale = cl.scales['12']['qual']['Paired']
custom_colorscale = base_colorscale * (num_unique_resources // len(base_colorscale)) + base_colorscale[:num_unique_resources % len(base_colorscale)]
fig = ff.create_gantt(tasks, index_col="Resource", show_colorbar=True, group_tasks=True, showgrid_x=True, colors=custom_colorscale)
fig.show()

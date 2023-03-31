log_file_name = ""
log_buffer = []
log_stdout = True


def init(file_name, stdout=True):
    global log_file_name
    global log_stdout

    log_file_name = file_name
    log_stdout = stdout

    with open(log_file_name, "w") as file:
        pass


def flush():
    global log_buffer
    global log_file_name

    with open(log_file_name, "a") as file:
        for line in log_buffer:
            file.write(line)
    log_buffer = []


def write(msg):
    global log_buffer
    global log_stdout

    if log_stdout:
        print(msg, end="")
    log_buffer.append(msg)
    if len(log_buffer) > 2048:
        flush()


def info(msg):
    write(f"[INFO]: {msg}\n")


def error(msg):
    write(f"[ERROR]: {msg}\n")


def warn(msg):
    write(f"[WARN]: {msg}\n")

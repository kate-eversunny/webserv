#!/usr/bin/python

import os


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


# |          Mode          |  r   |  r+  |  w   |  w+  |  a   |  a+  |
# | :--------------------: | :--: | :--: | :--: | :--: | :--: | :--: |
# |          Read          |  +   |  +   |      |  +   |      |  +   |
# |         Write          |      |  +   |  +   |  +   |  +   |  +   |
# |         Create         |      |      |  +   |  +   |  +   |  +   |
# |         Cover          |      |      |  +   |  +   |      |      |
# | Point in the beginning |  +   |  +   |  +   |  +   |      |      |
# |    Point in the end    |      |      |      |      |  +   |  +   |


def rreplace(s, old, new, occurrence):
    li = s.rsplit(old, occurrence)
    return new.join(li)


def multipart_save_file(boundary, path_body):
    log = os.environ.get("REMOTE_USER")
    with open(path_body, "r") as file_read:
        with open("./content/website1/users/" + log + "/avatar.png", "w") as new_file:
            list_lines = file_read.readlines()
            list_lines = list_lines[4:]
            new_file.writelines(list_lines[:list_lines.index("--" + boundary + "\r\n")])
    with open("./content/website1/authentication/authentication.txt", "r") as log_pas_read:
        text = log_pas_read.read()
    if len(text) > 0:
        d_log_pass = dict(x.split("=") for x in text.split(";"))
        if log in d_log_pass:
            password = d_log_pass[log][:-5]
            d_log_pass[log] = password + "true"
            save_list = [key + "=" + value for key, value in d_log_pass.items()]
            save_text = ";".join(save_list)
            with open("./content/website1/authentication/authentication.txt", "w") as log_pas_write:
                log_pas_write.write(save_text)
            return


def sign_up(log, password):
    with open("./content/website1/authentication/authentication.txt", "r+") as log_pas_write:
        text = log_pas_write.read()
        # print("sign up text", text)
        if len(text) > 0:
            d_log_pass = dict(x.split("=") for x in text.split(";"))
            # print(d_log_pass)
            if log in d_log_pass:
                print("login already exsists")
                return
        if len(text) > 0:
            log_pas_write.write(";")
        log_pas_write.write(log + "=" + password + "true")
        directory = "./content/website1/users/" + log
        if not os.path.exists(directory):
            os.makedirs(directory)


def login(log, password):
    with open("./content/website1/authentication/authentication.txt", "r") as log_pas_read:
        text = log_pas_read.read()
    if len(text) > 0:
        d_log_pass = dict(x.split("=") for x in text.split(";"))
        if log in d_log_pass and d_log_pass[log] == password + "false":
            d_log_pass[log] = password + "true"
            save_list = [key + "=" + value for key, value in d_log_pass.items()]
            save_text = ";".join(save_list)
            with open("./content/website1/authentication/authentication.txt", "w") as log_pas_write:
                log_pas_write.write(save_text)
            return
    print("wrong login or password")


def parse_input_data(input_data):
    dict_data = dict(x.split("=") for x in input_data.split("&"))  # {'loginSignUp': 'gbroccol', 'passwordSignUp': '1'}
    if "loginSignUp" in dict_data:
        sign_up(dict_data.get("loginSignUp"), dict_data.get("passwordSignUp"))
    elif "login" in dict_data:
        login(dict_data.get("login"), dict_data.get("password"))
    # elif "perform_logout" in dict_data: 
    # print headers


def main():
    print("I am in python")
    for value in os.environ:
        if os.environ[value].find("multipart/form-data; boundary=") >= 0:
            # print("save photo")
            multipart_save_file(os.environ[value][30:], os.environ.get("PATH_TRANSLATED"))
            return
    if "QUERY_STRING" in os.environ:
        # print("...=...?...=...")
        parse_input_data(os.environ.get("QUERY_STRING"))


main()


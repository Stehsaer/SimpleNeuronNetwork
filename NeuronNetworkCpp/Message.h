#pragma once

// common
#define MSG_UNKNOWN_CMD "Unknown command"
#define MSG_FUNC_NOT_IMPL "Command not implemented yet"
#define STR_CMD_START "\033[1;33m> \033[0m"

#define MSG_INVALID_INPUT "Invalid input"
#define MSG_INPUT_TOO_LARGE "Input too large"

#define MSG_STARTUP ">\033[1;37mNeuron Network Demo \033[0;37mby\033[1;34m Stehsaer\033[0m<\n\n"

// read network
#define MSG_READ_FILE_PATH "\033[1;36mFile path\033[0m > "
#define MSG_NO_SUCH_FILE "No such file"
#define MSG_READ_ERR "File corrupted"
#define MSG_CANT_OPEN_FILE "Can't open file"

// read dataset
#define MSG_READ_DATASET "\033[1;36mSelect slot (1~4)\033[0m > "
#define MSG_READ_IMAGE_DATASET "\033[1;36mImage path\033[0m > "
#define MSG_READ_LABEL_DATASET "\033[1;36mLabel path\033[0m > "
#define MSG_NORMALIZE_MODE "\033[1;36mNormalization mode\033[0m: (Press 0 for [0,1], Press 1 for [-1,1]) > "

// delete dataset
#define MSG_DELETE_DATASET "\033[1;36mSelect slot (1~4)\033[0m > "
#pragma once

int generate_auth_data(char* databuf);
unsigned int  get_auth_code(const char* authdata, const char* username);
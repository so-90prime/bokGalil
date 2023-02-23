#!/bin/sh


# +
# trap error(s)
# -
# set -o errexit
set -u nounset


# +
# single height
# -
write_ok () {
  # Use: write_ok "message" "OK"
  printf "\033[0m${1}\t[\033[0;32m${2}\033[0m]\n"
}

write_error () {
  # Use: write_error "message" "FAIL"
  printf "\033[0m${1}\t[\033[0;31m${2}\033[0m]\n"
}

write_blue () {
  # Use: write_blue "Blue"
  printf "\033[0;34m${1}\033[0m\n"
}

write_cyan () {
  # Use: write_cyan "Cyan"
  printf "\033[0;36m${1}\033[0m\n"
}

write_green () {
  # Use: write_green "Green"
  printf "\033[0;32m${1}\033[0m\n"
}

write_magenta () {
  # Use: write_magenta "Magenta"
  printf "\033[0;35m${1}\033[0m\n"
}

write_yellow () {
  # Use: write_yellow "Yellow"
  printf "\033[0;33m${1}\033[0m\n"
}

write_red () {
  # Use: write_red "Red"
  printf "\033[0;31m${1}\033[0m\n"
}

# +
# double height
# -
write_ok_dh () {
  # Use: write_ok_dh "message" "OK"
  printf "\033[0m\033#3${1}\t[\033[0;32m${2}\033[0m]\n\033[0m\033#4${1}\t[\033[0;32m${2}\033[0m]\n"
}

write_error_dh () {
  # Use: write_error_dh "message" "FAIL"
  printf "\033[0m\033#3${1}\t[\033[0;31m${2}\033[0m]\n\033[0m\033#4${1}\t[\033[0;31m${2}\033[0m]\n"
}

write_blue_dh () {
  # Use: write_blue_dh "Blue"
  printf "\033[0;34m\033#3${1}\n\033#4${1}\033[0m\n"
}

write_cyan_dh () {
  # Use: write_cyan_dh "Cyan"
  printf "\033[0;36m\033#3${1}\n\033#4${1}\033[0m\n"
}

write_green_dh () {
  # Use: write_green_dh "Green"
  printf "\033[0;32m\033#3${1}\n\033#4${1}\033[0m\n"
}

write_magenta_dh () {
  # Use: write_magenta_dh "Magenta"
  printf "\033[0;35m\033#3${1}\n\033#4${1}\033[0m\n"
}

write_red_dh () {
  # Use: write_red_dh "Red"
  printf "\033[0;31m\033#3${1}\n\033#4${1}\033[0m\n"
}

write_yellow_dh () {
  # Use: write_yellow_dh "Yellow"
  printf "\033[0;33m\033#3${1}\n\033#4${1}\033[0m\n"
}

# +
# other
# -
sha256sum () {
  # Use: sha256sum functions.sh
  openssl sha256 "$@" | awk '{print $2}'
}

replace_spaces () {
  # Use: replace_spaces The Quick Brown Fox Jumped Over The Lazy Dog
  echo $@ | sed 's/ /_/g'
}

remove_spaces () {
  # Use: remove_spaces The Quick Brown Fox Jumped Over The Lazy Dog
  echo $@ | sed 's/ //g'
}

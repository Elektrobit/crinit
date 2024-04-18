# SPDX-License-Identifier: MIT
#
# crinit-ctl bash completion script

_compreply_add() {
    while IFS='' read -r line || [[ -n ${line} ]];
        do COMPREPLY+=("${line}");
    done < <(eval "$1")
}

_add_static_options() {
    _compreply_add "compgen -W '$1' -- '${cur}'"
}

_add_fname_completions_filtered() {
    _compreply_add "compgen -o plusdirs -f -X '$1' -- '${cur}'"
}

_find_base_comp_words_index() {
    for i in "${!COMP_WORDS[@]}"; do
        if [[ "${COMP_WORDS[$i]}" == "$1" ]]; then
            echo "$i"
        fi
    done
}


_crinit-ctl() {
    local cur action
    compopt +o default
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    actindex=$(($(_find_base_comp_words_index "$1")+1))
    action="${COMP_WORDS[${actindex}]}"
    verbs="--help
        --version
        addtask
        addseries
        enable
        disable
        stop
        kill
        restart
        status
        notify
        list
        reboot
        poweroff"

    if [[ ${COMP_CWORD} -eq ${actindex} ]]; then
        _add_static_options "${verbs}"
        return 0
    fi

    case "${action}" in
        addtask)
            _add_static_options "--ignore-deps --override-deps --overwrite --verbose"
            _add_fname_completions_filtered "!*.crinit"
            ;;
        addseries)
            _add_static_options "--overwrite --verbose"
            _add_fname_completions_filtered "!*.series"
            ;;
        enable|disable|stop|kill|restart|status|notify)
            _add_static_options "--verbose $(crinit-ctl list | tail -n +2 | cut -f1 -d ' ')"
            ;;
        *)
            _add_static_options "--verbose"
    esac
    [[ ${COMP_CWORD} -ne ${actindex} ]] && [[ "${COMPREPLY[0]}" != "-*" ]] && compopt -o filenames
}

complete -F _crinit-ctl crinit-ctl

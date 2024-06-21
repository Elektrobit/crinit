# SPDX-License-Identifier: MIT
#
# crinit-ctl bash completion script
#
# Must be installed as `crinit-ctl` to a system directory where bash looks for completion
# scripts. It is likely to be `/usr/share/bash-completion/completions/crinit-ctl` on most
# modern distributions.
#

# Add entries to the completion reply array from compgen while avoiding splits
# on spaces and globbing. Necessary for e.g. filenames including whitespace.
_compreply_add() {
    while IFS='' read -r line || [[ -n ${line} ]];
        do COMPREPLY+=("${line}");
    done < <(eval "$1")
}

# Add a static multiline string to the completion dictionary, one entry per line.
_add_static_options() {
    _compreply_add "compgen -W '$1' -- '${cur}'"
}

# Add files/directories to the completion dictionary. Will use the directory position
# from the current completion input. Files will be filtered according to the given
# filter rule (see compgen manual).
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

# Main completion script, called on <TAB> on a crinit-ctl command line.
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
            _add_static_options "--verbose $(crinit-ctl list 2>/dev/null | tail -n +2 | cut -f1 -d ' ')"
            ;;
        *)
            _add_static_options "--verbose"
    esac
    [[ ${COMP_CWORD} -ne ${actindex} ]] && [[ "${COMPREPLY[0]}" != "-*" ]] && compopt -o filenames
}

complete -F _crinit-ctl crinit-ctl

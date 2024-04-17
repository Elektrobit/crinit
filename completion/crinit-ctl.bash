# SPDX-License-Identifier: MIT
#
# crinit-ctl bash completion script

_add_fname_completions_filtered() {
    paths=$(compgen -o plusdirs -f -X "$1" -- "${cur}")
    # line-by-line parsing necessary to handle whitespace filenames correctly
    while IFS= read -r entry || [[ -n ${entry} ]]; do
        COMPREPLY+=("${entry}")
    done < <(printf '%s' "${paths}")
}

_crinit-ctl() {
    local cur action opts
    compopt +o default
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    action="${COMP_WORDS[1]}"
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

    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${verbs}" -- "${cur}") )
        return 0
    fi

    case "${action}" in
        addtask)
            local opts="--ignore-deps --override-deps --overwrite"
            COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
            _add_fname_completions_filtered "!*.crinit"
            ;;
        addseries)
            local opts="--overwrite"
            COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
            _add_fname_completions_filtered "!*.series"
            ;;
        enable|disable|stop|kill|restart|status|notify)
            local tasks=$(crinit-ctl list | awk 'NR>1 { print $1 }' ORS=' ')
            COMPREPLY=( $(compgen -W "${tasks}" -- "${cur}") )
    esac
    [[ ${COMP_CWORD} -ne 1 ]] && [[ "${COMPREPLY}" != "-*" ]] && compopt -o filenames
}

complete -F _crinit-ctl crinit-ctl

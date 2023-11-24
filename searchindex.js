Search.setIndex({"docnames": ["API-Doc", "README", "doc/adr/ADR_template", "doc/adr/adr-crypt-lib", "doc/adr/adr-pipe-redirection", "doc/adr/adr-pubkey-storage", "doc/adr/adr-signature-algorithm", "doc/adr/adr-signature-conf-interface", "doc/adr/adr-signature-storage", "doc/adr/adr-task-include-strategy", "doc/adr/index", "index", "scripts/README", "src/index", "test/README"], "filenames": ["API-Doc.rst", "README.md", "doc/adr/ADR_template.md", "doc/adr/adr-crypt-lib.md", "doc/adr/adr-pipe-redirection.md", "doc/adr/adr-pubkey-storage.md", "doc/adr/adr-signature-algorithm.md", "doc/adr/adr-signature-conf-interface.md", "doc/adr/adr-signature-storage.md", "doc/adr/adr-task-include-strategy.md", "doc/adr/index.rst", "index.rst", "scripts/README.md", "src/index.rst", "test/README.md"], "titles": ["API Documentation", "Crinit \u2013 Configurable Rootfs Init", "Design Decisions - <headline>", "Architecture Design Record - Choice of cryptographic library for config signatures", "Architecture Design Record - Management of named pipes for IO redirection", "Architecture Design Record - Storage scheme for public keys", "Architecture Design Record - Choice of signature algorithm for signed configurations", "Architecture Design Record - Configuration interface for signature handling", "Architecture Design Record - Storage scheme for file signatures", "Architecture Design Record - INCLUDE file options", "Architecture Design Records", "Crinit \u2013 Configurable Rootfs Init", "Crinit-related scripts", "Developer", "Unit Tests"], "terms": {"our": [0, 3, 12], "doc": 0, "i": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12], "current": [0, 1, 3, 4, 6, 12], "avail": [0, 1, 3, 5], "doxygen": [0, 1], "format": [0, 1, 3, 8], "here": [0, 1, 8], "an": [1, 3, 4, 5, 6, 7, 9, 10, 12, 14], "daemon": 1, "execut": [1, 12], "pid": 1, "1": 1, "linux": [1, 3], "kernel": [1, 5, 8, 12], "It": [1, 7], "read": 1, "seri": 1, "either": [1, 8, 12], "specifi": 1, "command": [1, 8, 9, 12], "line": [1, 4, 8], "from": [1, 3, 4, 5, 6, 7, 9, 12, 14], "etc": 1, "default": [1, 4, 9, 12], "The": [1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 14], "turn": 1, "mai": [1, 4, 5, 6, 8, 9, 12], "refer": [1, 4], "further": [1, 6, 8], "whole": [1, 5], "directori": [1, 4, 14], "which": [1, 3, 5, 6, 8, 9, 10, 12], "all": [1, 3, 4, 6, 8, 14], "config": [1, 5, 8, 10], "shall": [1, 3, 4, 6, 7, 8, 9], "load": [1, 5, 8], "each": [1, 14], "one": [1, 4, 5, 9], "potenti": [1, 4], "contain": [1, 5, 8, 12, 14], "other": [1, 3, 6, 9, 12, 14], "ar": [1, 3, 4, 5, 6, 8, 9, 10, 12], "start": [1, 10], "much": [1, 4], "parallel": 1, "allow": [1, 9], "e": [1, 3, 5, 6, 12], "without": [1, 3], "ani": 1, "spawn": [1, 4, 9], "soon": 1, "possibl": [1, 4, 5, 6], "after": [1, 2, 4, 8], "ha": [1, 3, 6, 8, 12], "been": [1, 2, 3, 6, 12], "onc": [1, 8], "finish": 1, "fail": 1, "its": [1, 4, 8, 12, 14], "updat": [1, 5], "necessari": [1, 4, 6], "below": [1, 14], "diagram": 1, "show": [1, 12], "overal": 1, "Not": [1, 6], "indic": [1, 9], "plan": 1, "cryptograph": [1, 6, 10], "parser": [1, 7], "intend": [1, 9], "provid": [1, 3, 9, 10, 12], "option": [1, 10, 14], "verif": [1, 3, 8], "integr": [1, 4, 12], "check": [1, 12], "follow": [1, 2, 9, 10, 14], "implement": [1, 3, 4, 5, 6, 7, 9], "resolut": 1, "have": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12], "multipl": [1, 5, 7, 9], "none": 1, "form": [1, 4], "can": [1, 4, 5, 6, 7, 9, 10, 12, 14], "specif": 1, "state": [1, 6], "chang": [1, 5, 8, 9], "anoth": [1, 3, 4], "unknown": 1, "c": [1, 3, 14], "client": 1, "api": [1, 3, 11], "interfac": [1, 10, 12], "us": [1, 3, 4, 5, 6, 7, 8, 9, 11, 14], "capabl": 1, "ad": [1, 9], "new": [1, 4, 5, 6, 7, 9, 10], "manag": [1, 5, 10], "stop": 1, "kill": 1, "restart": 1, "alreadi": [1, 5, 7], "queri": 1, "statu": 1, "handl": [1, 4, 10], "reboot": 1, "poweroff": 1, "basic": [1, 4], "sourc": [1, 3], "compat": [1, 3], "sd_notifi": 1, "like": [1, 4, 6, 7, 8, 9], "shell": [1, 4], "log": 1, "purpos": [1, 5], "output": [1, 12, 14], "between": [1, 8, 12], "local": 1, "overrid": [1, 9], "extend": [1, 4], "preset": 1, "support": [1, 3, 4, 5, 6, 8, 9], "event": 1, "send": [1, 4], "addit": [1, 4, 6, 9, 14], "work": [1, 4, 9], "progress": 1, "abil": 1, "report": 1, "back": [1, 3], "signatur": [1, 5, 10, 12], "": [1, 4, 7, 12], "futur": 1, "we": [1, 3, 4, 6, 7, 8, 9, 10, 12], "also": [1, 3, 5, 6, 9, 12], "restrict": [1, 12], "process": [1, 2, 3, 4, 8], "uid": 1, "gid": 1, "cgroup": 1, "There": [1, 8, 10], "how": [1, 6, 7, 8, 9], "For": [1, 4], "inner": 1, "pleas": 1, "document": [1, 3, 9], "gener": [1, 6, 11], "dure": [1, 4, 8, 10], "h": [1, 12, 14], "share": [1, 3, 4], "librari": [1, 6, 10, 12, 14], "libcrinit": 1, "so": [1, 3, 4, 6], "thi": [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 14], "repositori": 1, "applic": 1, "machin": 1, "id": 1, "uniqu": 1, "identifi": [1, 8], "system": [1, 3, 4, 7, 9, 12], "elosd": 1, "gen": 1, "tool": [1, 12], "suppos": 1, "call": [1, 3], "boot": [1, 5, 7], "earlysetup": 1, "shown": [1, 14], "Its": 1, "valu": [1, 6, 9], "systemd": 1, "machine_id": 1, "given": [1, 4, 6, 8, 9, 12], "nxp": 1, "s32g": 1, "base": [1, 6, 9], "board": 1, "burn": 1, "chip": 1, "otp": 1, "memori": [1, 4], "If": [1, 3, 4, 5, 9, 12], "alwai": [1, 5, 7], "take": [1, 4, 6], "preced": 1, "physic": 1, "omit": 1, "mean": [1, 5, 6], "while": [1, 3, 4, 12], "special": [1, 4, 12], "function": [1, 3, 4, 6, 7, 9, 14], "soc": 1, "target": [1, 3, 4, 12], "long": [1, 6], "mit": 1, "copyright": 1, "2023": [1, 6, 12], "emlix": 1, "gmbh": 1, "elektrobit": 1, "automot": 1, "full": [1, 9], "text": 1, "found": 1, "root": [1, 8, 11], "softwar": [1, 12], "compani": [1, 3], "develop": [1, 3, 6, 9, 10, 11], "embed": [1, 3], "product": 1, "ecu": 1, "autosar": 1, "autom": 1, "drive": 1, "connect": [1, 4], "vehicl": 1, "ux": 1, "part": [1, 3], "corbo": 1, "built": [1, 14], "ubuntu": 1, "open": [1, 4, 10], "oper": 1, "high": [1, 3, 9], "perform": [1, 3, 6], "comput": [1, 6], "leverag": 1, "rich": 1, "meet": 1, "secur": [1, 3, 5, 6, 7, 9], "industri": 1, "regul": 1, "andrea": 1, "zdziarstek": 1, "com": [1, 3], "gmcn42": 1, "thoma": 1, "brinker": 1, "thomasbrink": 1, "schickedanz": 1, "anja": 1, "lehwess": 1, "litzmann": 1, "annika": 1, "schmitt": 1, "anton": 1, "hillebrand": 1, "daniel": 1, "gl\u00f6ckner": 1, "rainer": 1, "m\u00fcller": 1, "stefan": 1, "kral": 1, "wolfgang": 1, "gehrhardt": 1, "logo": 1, "swallow": 1, "hirundinida": 1, "quick": 1, "small": [1, 3], "bird": 1, "abl": [1, 10], "fly": 1, "distanc": 1, "origin": [1, 3, 6], "year": 1, "under": [1, 3], "creativ": 1, "common": [1, 14], "No": 1, "deriv": 1, "cc": 1, "nd": 1, "black": 1, "white": 1, "hks43": 1, "color": 1, "As": [1, 4, 6, 8], "describ": [1, 3, 6, 8], "abov": [1, 6, 8, 9, 12], "need": [1, 3, 4, 5, 7, 8, 9, 14], "well": [1, 6, 8, 12], "list": [1, 4, 9, 11], "demonstr": 1, "insid": [1, 3], "see": [1, 3, 6, 8, 9], "test": [1, 3, 7, 11], "point": [1, 4, 10], "minim": 1, "ini": 1, "style": [1, 8], "kei": [1, 6, 8, 10, 11], "pair": [1, 11], "some": [1, 3, 4, 5, 9, 12], "arrai": 1, "thei": [1, 4, 6, 9], "time": [1, 4, 6], "append": [1, 8, 9], "leav": 1, "out": [1, 3, 9], "favor": 1, "least": [1, 3, 4, 6, 9], "whitespac": 1, "charact": 1, "shorthand": 1, "last": 1, "array_like_kei": 1, "2": 1, "equival": [1, 6], "look": [1, 4, 6, 8, 9], "check_qemu": 1, "network": [1, 6, 9], "dhcp": 1, "sshd": 1, "getti": 1, "taskdir": 1, "task_file_suffix": 1, "taskdir_follow_symlink": 1, "ye": [1, 7, 9], "includedir": 1, "include_suffix": 1, "crincl": [1, 9], "debug": 1, "NO": [1, 9], "shutdown_grace_period_u": 1, "100000": 1, "use_syslog": 1, "use_elo": 1, "elos_serv": 1, "192": 1, "168": 1, "3": 1, "43": 1, "elos_port": 1, "2342": 1, "env_set": [1, 9], "foo": [1, 9], "foo_baz": 1, "baz": [1, 9], "greet": 1, "good": [1, 3, 9], "morn": 1, "scan": 1, "where": [1, 3, 4, 9], "find": [1, 4, 7], "prepend": [1, 9], "filenam": [1, 12], "suffix": 1, "onli": [1, 5, 6, 8, 9, 12], "relev": [1, 3], "symbol": 1, "link": [1, 3, 6, 8], "should": [1, 3, 4, 5, 6, 7, 8, 10], "referenc": 1, "same": [1, 3, 4, 6, 8, 9, 12], "verbos": 1, "amount": [1, 3], "microsecond": 1, "wait": [1, 4, 9], "sigterm": 1, "sigkil": 1, "shutdown": [1, 4], "syslog": 1, "switch": 1, "ideal": 1, "server": [1, 9], "syslogd": 1, "prefix": 1, "ip": 1, "address": 1, "127": 1, "0": [1, 4], "port": 1, "54321": 1, "section": 1, "filter_defin": 1, "could": [1, 3, 5, 9, 12], "daemon_env_preset": 1, "bin": 1, "mkdir": 1, "p": 1, "var": 1, "lib": 1, "dhcpcd": 1, "mount": 1, "t": 1, "tmpf": 1, "touch": [1, 8], "resolv": 1, "conf": 1, "o": [1, 12], "bind": 1, "sbin": 1, "ifconfig": 1, "lo": 1, "up": [1, 3, 4, 5], "j": 1, "eth0": 1, "writable_var": 1, "ipv4_dhcp": 1, "resolvconf": 1, "respawn": [1, 9], "respawn_retri": 1, "foo_bar": 1, "bar": [1, 9], "escaped_var": 1, "var_with_esc_sequ": 1, "hex": 1, "x68": 1, "x65": 1, "x78": 1, "even": [1, 3, 4, 5], "io_redirect": [1, 4], "stdout": [1, 4], "net": 1, "0644": 1, "stderr": 1, "want": [1, 4, 7], "mandatori": [1, 12], "path": [1, 4, 9], "must": [1, 5, 12], "absolut": [1, 4], "them": [1, 5, 8, 9, 10, 12], "consid": [1, 10], "fulfil": 1, "successfulli": 1, "return": 1, "treat": 1, "befor": [1, 3], "readi": 1, "semant": 1, "tasknam": 1, "when": 1, "first": [1, 4, 5, 12], "complet": [1, 3, 5, 7], "somewher": 1, "along": 1, "wai": [1, 4, 7, 12], "run": [1, 4, 14], "setup": [1, 12], "fulli": 1, "determin": 1, "emul": 1, "therefor": 1, "exit": 1, "error": [1, 14], "code": [1, 3, 7], "left": [1, 2], "explicitli": 1, "empti": 1, "interpret": 1, "syntax": 1, "case": [1, 4, 9, 12, 14], "writabl": [1, 4], "partit": 1, "That": 1, "would": [1, 3, 4, 7, 8, 12], "advertis": 1, "addition": [1, 4], "issu": [1, 3], "filter_nam": 1, "fullfil": 1, "trigger": 1, "themself": 1, "keyword": [1, 4], "seen": [1, 4], "g": [1, 3, 6, 12], "chosen": [1, 3, 6], "reflect": 1, "intent": [1, 4], "arbitrarili": [1, 8], "failur": 1, "number": [1, 5, 6], "row": 1, "again": 1, "unlimit": 1, "valid": [1, 9, 12], "overriden": [1, 9], "togeth": 1, "result": [1, 6], "explanatori": 1, "comment": 1, "expans": 1, "avoid": 1, "through": 1, "escap": 1, "sequenc": 1, "hexadecim": 1, "byte": 1, "enabl": [1, 6], "logger": 1, "order": [1, 4, 5, 9], "conjunct": [1, 5], "correspond": [1, 8, 14], "within": [1, 4], "itself": [1, 4, 5], "definit": [1, 7], "normal": [1, 14], "instead": [1, 12], "filter_rul": 1, "elos_ssh_event_task": 1, "echo": 1, "sshd_filter": 1, "appnam": 1, "strcmp": 1, "statement": [1, 4], "singl": [1, 3, 9], "variable_nam": 1, "content": 1, "quot": 1, "around": 1, "appear": 1, "twice": 1, "instanc": 1, "expand": 1, "b": 1, "n": 1, "x": 1, "two": [1, 4, 7], "digit": [1, 6], "effect": [1, 9], "manual": 1, "copi": 1, "exactli": 1, "similar": [1, 4, 5, 6, 7, 8, 9], "import": [1, 9], "appli": [1, 6], "include_nam": 1, "end": 1, "lead": [1, 9], "comma": 1, "separ": [1, 3, 14], "everyth": [1, 5], "taken": [1, 2, 3, 4, 5, 6, 7, 8, 9], "server_set": 1, "assum": [1, 12], "dir": 1, "http_port": 1, "8080": 1, "txt": 1, "ignor": 1, "per": 1, "IN": 1, "err": [1, 4], "redirect_from": 1, "redirect_to": 1, "truncat": 1, "octal_mod": [1, 4], "stdin": [1, 4], "those": [1, 3], "stream": [1, 3, 4], "signifi": 1, "whether": 1, "exist": [1, 3, 4, 5, 7, 8], "locat": [1, 4], "discuss": [1, 10], "permiss": [1, 3], "bit": [1, 6, 12], "newli": 1, "creat": [1, 4, 12], "accordingli": 1, "being": [1, 8], "mode": [1, 3], "doe": [1, 3, 4, 8, 9], "yet": [1, 6, 12], "second": 1, "captur": 1, "both": 1, "dev": 1, "null": 1, "silenc": 1, "opt": [1, 14], "data": [1, 8, 12], "backup": 1, "tar": 1, "gz": 1, "go": 1, "consol": 1, "accept": [1, 7], "With": [1, 4], "ensur": [1, 5], "fifo": 1, "sender": [1, 4], "receiv": [1, 4], "util": 1, "busybox": 1, "some_task": [1, 8], "tmp": [1, 4], "some_task_log_pip": 1, "0640": 1, "some_task_logg": 1, "usr": 1, "input": [1, 12], "therebi": [1, 9], "kept": 1, "mind": 1, "keep": [1, 5], "differ": [1, 3, 6, 7, 8, 12], "race": 1, "condit": 1, "access": [1, 4], "By": 1, "glibc": 1, "block": 1, "make": [1, 3, 12], "hard": [1, 5], "tail": 1, "f": 1, "monitor": 1, "To": [1, 14], "get": [1, 9], "problem": [1, 10], "stdbuf": 1, "gnu": [1, 3], "coreutil": 1, "line_buffered_task": 1, "ol": 1, "el": 1, "some_execut": 1, "els": 1, "easili": [1, 4], "more": [1, 4, 5, 7, 8, 9, 12, 14], "man": [1, 4, 5], "page": 1, "immedi": [1, 8], "own": [1, 4, 8, 14], "combin": [1, 6], "reason": [1, 4, 5, 7, 8], "do": [1, 4, 5, 12], "readabl": 1, "hook": 1, "third": [1, 5, 9, 12], "parti": [1, 5, 9, 12], "opaqu": [1, 9], "view": 1, "dep_grp_serv": 1, "sql": [1, 9], "db": [1, 9], "firewal": 1, "httpd": [1, 9], "local_http_cli": 1, "http": 1, "request": 1, "localhost": 1, "care": 1, "about": [1, 3], "singular": 1, "entiti": 1, "deliv": 1, "knowledg": [1, 9], "you": [1, 12], "what": [1, 9], "who": 1, "cli": 1, "control": [1, 9], "program": 1, "wrap": [1, 3], "help": [1, 12], "action": 1, "paramet": 1, "addtask": 1, "overwrit": 1, "dep": [1, 9], "d": 1, "depa": 1, "eventa": 1, "depb": 1, "eventb": 1, "Will": [1, 12], "add": [1, 4, 5], "databas": 1, "let": [1, 4], "know": 1, "fine": 1, "field": 1, "shortcut": 1, "addseri": 1, "over": [1, 9], "task_nam": 1, "remov": 1, "present": [1, 3, 6, 7], "disabl": 1, "known": [1, 5], "reset": 1, "done": [1, 3, 7, 12], "notifi": 1, "sd_notify_str": 1, "mainpid": 1, "print": 1, "grace": 1, "symlink": 1, "invok": 1, "automat": 1, "v": 1, "Be": 1, "version": [1, 4, 6, 12], "inform": [1, 3, 6, 11], "success": [1, 14], "ci": [1, 3, 14], "docker": 1, "sh": [1, 11, 14], "nativ": 1, "host": 1, "architectur": [1, 14], "short": 1, "foreign": 1, "arm64": 1, "qemu": 1, "user": [1, 4, 8, 9, 12], "static": [1, 3], "binfmt": 1, "sure": [1, 3, 12], "packag": [1, 7], "instal": [1, 7], "your": 1, "regardless": 1, "jammi": 1, "desir": [1, 3], "lunar": 1, "amd64": 1, "suffici": 1, "compil": [1, 5, 7, 12], "releas": [1, 3, 4, 6], "suit": [1, 3], "rpm": 1, "script": [1, 11, 14], "artifact": 1, "addresssanit": 1, "asan": 1, "runtim": [1, 7], "analysi": 1, "fanalyz": 1, "analyz": 1, "afterward": [1, 9], "demo": 1, "clang": 1, "tidi": 1, "compile_command": 1, "json": 1, "save": 1, "unit": 1, "smoke": 1, "respect": [1, 12], "argument": [1, 12], "utest": [1, 14], "smoketest": 1, "robot": 1, "framework": 1, "rune": 1, "adapt": 1, "robot_vari": 1, "py": 1, "debian": 1, "debbuild": 1, "binari": [1, 5, 7, 8, 14], "cmake": [1, 14], "dcmake_build_typ": 1, "dcmake_verbose_makefil": 1, "On": [1, 14], "dunit_test": [1, 14], "alon": 1, "constraint": 2, "account": 2, "suitabl": [2, 6], "solut": [2, 9], "made": [2, 6, 10, 12], "descript": [2, 10], "pro": 2, "But": [2, 3, 4, 6, 9], "con": 2, "42": 2, "someth": [2, 4], "wa": [2, 3, 12], "crinit": [3, 4, 5, 6, 7, 8, 9, 10, 14], "sign": [3, 7, 8, 10, 11], "task": [3, 4, 6, 7, 12], "global": [3, 4, 6, 12], "configur": [3, 10], "algorithm": [3, 10], "rsa": [3, 12], "pss": [3, 12], "adr": [3, 11], "choos": [3, 6, 9], "right": [3, 6], "offer": [3, 4, 7], "sha256": [3, 6, 8], "4096": [3, 6, 12], "term": [3, 6], "rsassa": [3, 6], "interchang": [3, 6], "literatur": [3, 6], "thing": [3, 6], "minimalist": 3, "wide": [3, 6], "devic": [3, 5], "os": 3, "activ": [3, 7], "regular": 3, "modern": 3, "hyoertext": 3, "7": [3, 5], "larg": [3, 6], "organ": 3, "github": 3, "merg": [3, 9], "via": [3, 5, 8], "8": 3, "licens": [3, 11], "apach": 3, "modular": 3, "size": 3, "feasibl": 3, "fip": [3, 6], "140": 3, "certif": 3, "effort": [3, 4, 6, 9], "now": [3, 4], "featur": [3, 9, 11], "pack": 3, "divers": 3, "rang": 3, "longest": 3, "histori": 3, "among": 3, "veri": [3, 6], "huge": 3, "set": [3, 12, 14], "manpag": 3, "6": [3, 6], "extens": [3, 7, 9], "sinc": 3, "v3": 3, "certifi": 3, "reduc": 3, "rel": 3, "usual": [3, 4], "most": [3, 4], "anywai": [3, 4, 9], "due": [3, 6], "depend": [3, 7, 9, 12], "complex": [3, 4, 8, 9], "codebas": 3, "evidenc": 3, "regress": 3, "far": [3, 8], "solv": 3, "corpor": 3, "name": [3, 9, 10, 14], "paid": 3, "edit": [3, 6], "older": 3, "than": [3, 4, 6, 8, 9], "modul": [3, 5], "free": 3, "websit": 3, "complianc": 3, "suggest": [3, 4, 9], "area": [3, 4], "might": [3, 4, 9], "freeli": 3, "non": [3, 6], "gpl": 3, "seem": [3, 9, 12], "lean": 3, "2mb": 3, "file": [3, 5, 10, 11], "replac": 3, "project": 3, "whose": 3, "deem": 3, "incompat": 3, "split": 3, "off": [3, 4], "main": [3, 9, 14], "lgpl": 3, "5": [3, 6], "aim": 3, "compliant": 3, "actual": 3, "red": 3, "hat": 3, "enterpris": 3, "profil": 3, "gnome": 3, "less": [3, 4, 8], "exampl": [3, 5, 9, 11], "mostli": [3, 5], "commun": 3, "note": [3, 6, 11], "level": [3, 4], "rest": [3, 8], "easi": [3, 5, 7, 8], "swap": 3, "against": [3, 6], "plianc": 3, "build": [3, 11, 14], "object": 3, "rebuild": 3, "16791": 3, "17064": 3, "wolfcrypt": 3, "9": 4, "simplifi": 4, "prior": [4, 12], "ti": 4, "themselv": 4, "mkfifo": 4, "remain": 4, "question": [4, 9], "somehow": 4, "lifetim": 4, "enough": [4, 6], "presenc": [4, 5, 12], "temporari": 4, "entir": 4, "In": [4, 5, 6, 9, 11, 12], "stai": 4, "until": [4, 10], "extern": [4, 12], "delet": 4, "simpl": [4, 7, 9], "concept": [4, 9, 11], "requir": [4, 6, 7, 9], "structur": 4, "central": 4, "resid": [4, 12, 14], "organis": 4, "offload": 4, "maintain": [4, 7, 9, 11], "sender_task": 4, "receiver_task": 4, "upon": 4, "encount": 4, "fifodir": 4, "clear": [4, 8], "termin": 4, "better": 4, "outsid": 4, "behav": 4, "otherwis": [4, 6, 9], "exclud": 4, "consider": 4, "intuit": [4, 9], "approach": 4, "schema": 4, "descriptor": 4, "simpler": 4, "especi": [4, 9], "clean": 4, "analogu": 4, "startabl": 4, "improv": [4, 9], "speed": [4, 6], "henc": 4, "simlarli": 4, "mileston": 4, "later": 4, "technic": [4, 6], "groundwork": 4, "low": [4, 9], "higher": 4, "logic": 4, "step": [4, 12], "stone": 4, "evalu": 4, "feedback": 4, "toward": [4, 6], "down": 4, "track": 5, "These": [5, 12], "protect": 5, "tamper": 5, "authent": [5, 6], "trust": [5, 8, 12], "vendor": [5, 12], "tree": 5, "dtb": 5, "initram": 5, "disk": [5, 12], "A": [5, 6, 8, 14], "plain": [5, 9], "trustworthi": 5, "defeat": 5, "individu": 5, "place": [5, 8, 10, 12], "unsign": 5, "retent": 5, "facil": 5, "method": 5, "chain": [5, 7], "defin": [5, 9], "hardwar": [5, 6], "proof": 5, "privat": [5, 6, 12], "critic": 6, "usabl": [6, 7, 9], "recommend": 6, "bsi": 6, "guidelin": 6, "tr": 6, "02102": 6, "classic": 6, "probabilist": 6, "scheme": [6, 7, 10, 14], "mathemat": 6, "prove": 6, "strength": 6, "tightli": 6, "relat": [6, 11, 14], "underli": 6, "understood": 6, "length": 6, "hash": [6, 8, 12], "approv": 6, "nist": 6, "186": 6, "computation": 6, "expens": 6, "domin": 6, "distinct": 6, "publish": 6, "1991": 6, "u": 6, "government": 6, "agenc": 6, "usag": [6, 9, 11], "public": [6, 8, 10, 11, 12], "cryptographi": [6, 12], "anymor": 6, "recent": [6, 12], "ellipt": 6, "curv": 6, "shorter": 6, "theoret": 6, "faster": 6, "realli": 6, "concern": 6, "practic": 6, "acceler": 6, "quit": 6, "polit": 6, "voic": 6, "wake": 6, "nsa": 6, "scandal": 6, "reveal": 6, "backdoor": 6, "random": 6, "dual_ec_drbg": 6, "cast": 6, "unproven": 6, "doubt": 6, "ssh": 6, "move": 6, "ed25519": 6, "reli": [6, 9], "asymmetr": 6, "quantum": 6, "attack": 6, "limit": [6, 12], "proport": 6, "cut": 6, "edg": 6, "crypto": 6, "still": 6, "mention": [6, 8], "dss": 6, "beyond": 6, "3000": 6, "pad": 6, "salt": 6, "accord": 6, "pkc": 6, "standard": [6, 12], "mechan": 6, "01": 6, "feder": 6, "offic": 6, "pdf": 6, "bellar": 6, "mihir": 6, "phillip": 6, "rogawai": 6, "1996": 6, "exact": 6, "rabin": 6, "advanc": [6, 9], "cryptologi": 6, "eurocrypt": 6, "96": 6, "ueli": 6, "maurer": 6, "399": 6, "416": 6, "berlin": 6, "heidelberg": 6, "springer": 6, "nation": 6, "institut": 6, "technologi": 6, "butin": 6, "deni": 6, "julian": 6, "w\u00e4lde": 6, "johann": 6, "buchmann": 6, "2017": 6, "post": 6, "openssl": [6, 12], "tenth": 6, "intern": [6, 12], "confer": 6, "mobil": 6, "ubiquit": 6, "icmu": 6, "expect": [7, 8, 14], "implic": 7, "deactiv": 7, "caus": 7, "complic": 7, "verifi": [7, 8], "imposs": [7, 9], "unverifi": 7, "flexibl": [7, 8, 9], "getopt": 7, "unusu": 7, "init": 7, "awkward": 7, "bootload": 7, "perspect": [7, 9], "implicitli": 7, "re2c": 7, "decid": 8, "store": 8, "ring": 8, "filesystem": 8, "keyid": 8, "includ": [8, 10, 14], "particular": 8, "why": 8, "consum": 8, "too": 8, "mani": [8, 10], "cpu": 8, "cycl": 8, "calcul": 8, "ram": 8, "keyr": [8, 12], "100": 8, "ascii": 8, "plaintext": 8, "uuencod": 8, "necessarili": 8, "slightli": 8, "confus": 8, "sig": [8, 12], "mesh": 8, "appar": 8, "templat": [9, 10], "base_env_vari": 9, "added_env_vari": 9, "compound_var": 9, "variabl": 9, "best": 9, "eas": 9, "circumvent": 9, "happen": 9, "multi": 9, "prohibit": 9, "downstream": [9, 11], "side": 9, "becom": 9, "chaotic": 9, "involv": 9, "problemat": 9, "behavior": 9, "big": 9, "greatest": 9, "esp": 9, "propag": 9, "safe": 9, "filter": 9, "encourag": 9, "tabl": 9, "idea": [9, 10], "numer": 9, "anyhow": 9, "advic": 9, "nice": 9, "decis": 10, "chapter": 10, "collect": 10, "agre": 10, "final": 10, "topic": 10, "roughli": 10, "pattern": 10, "influenc": 10, "factor": 10, "assumpt": 10, "altern": [10, 12], "ration": 10, "choic": 10, "pipe": 10, "io": 10, "redirect": 10, "storag": 10, "nutshel": 11, "power": 11, "eb": 11, "credit": 11, "artwork": 11, "detail": 11, "ctl": 11, "info": 11, "instruct": 11, "genkei": 11, "type": 11, "hierarchi": 11, "subfold": 12, "mainten": 12, "mbedtl": 12, "space": 12, "k": 12, "key_fil": 12, "output_fil": 12, "obtain": 12, "write": 12, "input_fil": 12, "sha": 12, "256": 12, "posit": 12, "Then": 12, "hsm": 12, "pub": 12, "suppli": 12, "wish": 12, "upstream": 12, "li": 12, "metadata": 12, "flag": 12, "pre": 12, "appropri": 12, "bug": 12, "prevent": 12, "pass": [12, 14], "fix": 12, "march": 12, "22": 12, "2022": 12, "juli": 12, "stabl": 12, "distribut": 12, "placehold": 13, "top": 14, "aarch64": 14, "produc": 14, "mock": 14, "glob": 14, "crinitgloboptset": 14, "glot": 14, "header": 14, "ch": 14, "crinitfunctionnam": 14, "testcas": 14, "testsuit": 14, "declar": 14, "unit_test": 14, "cmocka": 14}, "objects": {}, "objtypes": {}, "objnames": {}, "titleterms": {"api": 0, "document": 0, "crinit": [1, 11, 12], "configur": [1, 4, 6, 7, 8, 9, 11, 12], "rootf": [1, 5, 11], "init": [1, 11], "In": 1, "nutshel": 1, "concept": 1, "featur": 1, "licens": 1, "power": 1, "eb": 1, "maintain": 1, "credit": 1, "artwork": 1, "detail": 1, "exampl": [1, 12], "global": [1, 7], "explan": 1, "task": [1, 9], "set": [1, 9], "environ": 1, "variabl": 1, "defin": 1, "elo": 1, "filter": 1, "ruleset": 1, "includ": [1, 9], "file": [1, 4, 8, 9, 12, 14], "io": [1, 4], "redirect": [1, 4], "name": [1, 4, 8], "pipe": [1, 4], "A": 1, "note": [1, 12], "buffer": 1, "depend": 1, "group": 1, "meta": 1, "ctl": 1, "usag": [1, 12], "info": 1, "build": [1, 7], "instruct": 1, "design": [2, 3, 4, 5, 6, 7, 8, 9, 10], "decis": [2, 3, 4, 5, 6, 7, 8, 9], "headlin": 2, "problem": [2, 3, 4, 5, 6, 7, 8, 9], "influenc": [2, 3, 4, 5, 6, 7, 9], "factor": [2, 3, 4, 5, 6, 7, 9], "assumpt": [2, 3, 4, 5, 6, 7, 8], "consid": [2, 3, 4, 5, 6, 7, 8, 9], "altern": [2, 3, 4, 5, 6, 7, 8, 9], "1": [2, 3, 4, 5, 6, 7, 8, 9], "alt": 2, "2": [2, 3, 4, 5, 6, 7, 8, 9], "3": [2, 3, 4, 5, 6, 7, 8, 9], "rational": [2, 4], "open": 2, "point": 2, "architectur": [3, 4, 5, 6, 7, 8, 9, 10], "record": [3, 4, 5, 6, 7, 8, 9, 10], "choic": [3, 6], "cryptograph": 3, "librari": 3, "config": 3, "signatur": [3, 6, 7, 8], "option": [3, 4, 5, 6, 7, 8, 9], "mbedtl": 3, "pro": [3, 4, 5, 6, 7, 8, 9], "con": [3, 4, 5, 6, 7, 8, 9], "openssl": 3, "version": 3, "0": 3, "later": 3, "wolfssl": 3, "4": [3, 4, 6, 7], "gnutl": 3, "refer": [3, 6], "manag": 4, "aka": 4, "fifo": 4, "creation": 4, "onli": 4, "both": 4, "usabl": 4, "decid": 4, "through": 4, "anonym": 4, "classic": 4, "storag": [5, 8], "scheme": [5, 8], "public": 5, "kei": [5, 12], "directori": 5, "system": 5, "keyr": 5, "master": 5, "sign": [5, 6, 12], "algorithm": 6, "rsa": 6, "ssa": 6, "pss": 6, "dsa": 6, "ecdsa": 6, "merkl": 6, "interfac": 7, "handl": 7, "time": 7, "command": 7, "line": 7, "argument": 7, "pars": 7, "kernel": 7, "within": 8, "separ": 8, "associ": 8, "path": 8, "all": 9, "possibl": 9, "pre": 9, "polici": 9, "subset": 9, "limit": 9, "number": 9, "list": 10, "adr": 10, "content": [10, 11], "relat": 12, "script": 12, "inform": 12, "genkei": 12, "sh": 12, "gener": 12, "root": 12, "pair": 12, "downstream": 12, "type": 12, "us": 12, "develop": 13, "unit": 14, "test": 14, "hierarchi": 14}, "envversion": {"sphinx.domains.c": 3, "sphinx.domains.changeset": 1, "sphinx.domains.citation": 1, "sphinx.domains.cpp": 9, "sphinx.domains.index": 1, "sphinx.domains.javascript": 3, "sphinx.domains.math": 2, "sphinx.domains.python": 4, "sphinx.domains.rst": 2, "sphinx.domains.std": 2, "sphinx.ext.viewcode": 1, "sphinx": 60}, "alltitles": {"API Documentation": [[0, "api-documentation"]], "Crinit \u2013 Configurable Rootfs Init": [[1, "crinit-configurable-rootfs-init"], [11, "crinit-configurable-rootfs-init"]], "In a Nutshell": [[1, "in-a-nutshell"]], "Concept": [[1, "concept"]], "Features": [[1, "features"]], "License": [[1, "license"]], "Powered by EB": [[1, "powered-by-eb"]], "Maintainers": [[1, "maintainers"]], "Credits": [[1, "credits"]], "Artwork": [[1, "artwork"]], "Details": [[1, "details"]], "Configuration": [[1, "configuration"]], "Example Global Configuration": [[1, "example-global-configuration"]], "Explanation": [[1, "explanation"], [1, "id1"]], "Example Task Configuration": [[1, "example-task-configuration"]], "Setting Environment Variables": [[1, "setting-environment-variables"]], "Defining Elos Filters": [[1, "defining-elos-filters"]], "Ruleset": [[1, "ruleset"]], "Include files": [[1, "include-files"]], "IO Redirections": [[1, "io-redirections"]], "Named pipes": [[1, "named-pipes"]], "A note on buffering": [[1, "a-note-on-buffering"]], "Dependency groups (meta-tasks)": [[1, "dependency-groups-meta-tasks"]], "crinit-ctl Usage Info": [[1, "crinit-ctl-usage-info"]], "Build Instructions": [[1, "build-instructions"]], "Design Decisions - <headline>": [[2, "design-decisions"]], "Problem": [[2, "problem"], [3, "problem"], [4, "problem"], [5, "problem"], [6, "problem"], [7, "problem"], [8, "problem"], [9, "problem"]], "Influencing factors": [[2, "influencing-factors"]], "Assumptions": [[2, "assumptions"], [3, "assumptions"], [4, "assumptions"], [5, "assumptions"], [6, "assumptions"], [7, "assumptions"]], "Considered Alternatives": [[2, "considered-alternatives"], [3, "considered-alternatives"], [4, "considered-alternatives"], [5, "considered-alternatives"], [6, "considered-alternatives"], [7, "considered-alternatives"], [8, "considered-alternatives"], [9, "considered-alternatives"]], "1) <alt 1>": [[2, "alt-1"]], "2) <alt 2>": [[2, "alt-2"]], "3) <alt 3>": [[2, "alt-3"]], "Decision": [[2, "decision"], [3, "decision"], [4, "decision"], [5, "decision"], [6, "decision"], [7, "decision"], [8, "decision"], [9, "decision"]], "Rationale": [[2, "rationale"], [4, "rationale"]], "Open Points": [[2, "open-points"]], "Architecture Design Record - Choice of cryptographic library for config signatures": [[3, "architecture-design-record-choice-of-cryptographic-library-for-config-signatures"]], "Influencing Factors": [[3, "influencing-factors"], [4, "influencing-factors"], [5, "influencing-factors"], [6, "influencing-factors"], [7, "influencing-factors"], [9, "influencing-factors"]], "Option 1 - mbedTLS": [[3, "option-1-mbedtls"]], "Pros": [[3, "pros"], [3, "id3"], [3, "id8"], [3, "id12"], [4, "pros"], [4, "id1"], [4, "id3"], [4, "id5"], [5, "pros"], [5, "id1"], [5, "id3"], [6, "pros"], [6, "id4"], [6, "id7"], [6, "id10"], [7, "pros"], [7, "id1"], [7, "id3"], [7, "id5"], [8, "pros"], [8, "id1"], [8, "id3"], [9, "pros"], [9, "id1"], [9, "id3"]], "Cons": [[3, "cons"], [3, "id5"], [3, "id11"], [3, "id14"], [4, "cons"], [4, "id2"], [4, "id4"], [4, "id6"], [5, "cons"], [5, "id2"], [5, "id4"], [6, "cons"], [6, "id5"], [6, "id9"], [6, "id12"], [7, "cons"], [7, "id2"], [7, "id4"], [7, "id6"], [8, "cons"], [8, "id2"], [8, "id4"], [9, "cons"], [9, "id2"], [9, "id4"]], "Option 2 - OpenSSL (version 3.0.0 or later)": [[3, "option-2-openssl-version-3-0-0-or-later"]], "Option 3 - wolfSSL": [[3, "option-3-wolfssl"]], "Option 4 - GnuTLS": [[3, "option-4-gnutls"]], "References": [[3, "references"], [6, "references"]], "Architecture Design Record - Management of named pipes for IO redirection": [[4, "architecture-design-record-management-of-named-pipes-for-io-redirection"]], "Option 1 - Named Pipe (aka FIFO) creation only": [[4, "option-1-named-pipe-aka-fifo-creation-only"]], "Option 2 - Pipe file management": [[4, "option-2-pipe-file-management"]], "Option 3 - Both usable, decided through configuration": [[4, "option-3-both-usable-decided-through-configuration"]], "Option 4 - Anonymous \u2018classic\u2019 pipes": [[4, "option-4-anonymous-classic-pipes"]], "Architecture Design Record - Storage scheme for public keys": [[5, "architecture-design-record-storage-scheme-for-public-keys"]], "Option 1 - Key directory in rootfs": [[5, "option-1-key-directory-in-rootfs"]], "Option 2 - Keys in system keyring": [[5, "option-2-keys-in-system-keyring"]], "Option 3 - Master key in system keyring, signed keys in rootfs key directory": [[5, "option-3-master-key-in-system-keyring-signed-keys-in-rootfs-key-directory"]], "Architecture Design Record - Choice of signature algorithm for signed configurations": [[6, "architecture-design-record-choice-of-signature-algorithm-for-signed-configurations"]], "Option 1 - RSA(SSA)-PSS": [[6, "option-1-rsa-ssa-pss"]], "Option 2 - DSA": [[6, "option-2-dsa"]], "Option 3 - ECDSA": [[6, "option-3-ecdsa"]], "Option 4 - Merkle signatures": [[6, "option-4-merkle-signatures"]], "Architecture Design Record - Configuration interface for signature handling": [[7, "architecture-design-record-configuration-interface-for-signature-handling"]], "Option 1 - Configuration at build-time": [[7, "option-1-configuration-at-build-time"]], "Option 2 - Configured in global configuration": [[7, "option-2-configured-in-global-configuration"]], "Option 3 - command line argument": [[7, "option-3-command-line-argument"]], "Option 4 - parse Kernel command line": [[7, "option-4-parse-kernel-command-line"]], "Architecture Design Record - Storage scheme for file signatures": [[8, "architecture-design-record-storage-scheme-for-file-signatures"]], "Assumption": [[8, "assumption"]], "Option 1 - Within the configuration file": [[8, "option-1-within-the-configuration-file"]], "Option 2 - Separate file associated by name": [[8, "option-2-separate-file-associated-by-name"]], "Option 3 - Separate file associated by configurable path": [[8, "option-3-separate-file-associated-by-configurable-path"]], "Architecture Design Record - INCLUDE file options": [[9, "architecture-design-record-include-file-options"]], "Option 1 - All options possible, pre-set policy": [[9, "option-1-all-options-possible-pre-set-policy"]], "Option 2 - All options possible with task-configurable subset": [[9, "option-2-all-options-possible-with-task-configurable-subset"]], "Option 3 - Limited number of options possible with task-configurable subset": [[9, "option-3-limited-number-of-options-possible-with-task-configurable-subset"]], "Architecture Design Records": [[10, "architecture-design-records"]], "List of ADRs": [[10, "list-of-adrs"]], "Contents:": [[10, null]], "Contents": [[11, null]], "Crinit-related scripts": [[12, "crinit-related-scripts"]], "Script usage information": [[12, "script-usage-information"]], "crinit-genkeys.sh": [[12, "crinit-genkeys-sh"]], "crinit-sign.sh": [[12, "crinit-sign-sh"]], "Usage examples": [[12, "usage-examples"]], "Generate a root key pair": [[12, "generate-a-root-key-pair"]], "Generate downstream keys signed by the root key": [[12, "generate-downstream-keys-signed-by-the-root-key"]], "Sign configuration files": [[12, "sign-configuration-files"]], "Note on the key type used in crinit-genkeys.sh": [[12, "note-on-the-key-type-used-in-crinit-genkeys-sh"]], "Developer": [[13, "developer"]], "Unit Tests": [[14, "unit-tests"]], "File Hierarchy": [[14, "file-hierarchy"]]}, "indexentries": {}})
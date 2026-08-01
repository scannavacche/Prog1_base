#!/usr/bin/env bash
# Test completi per ./musica su canzoni.csv
# Uso: chmod +x test_musica.sh && ./test_musica.sh
# Override: EXE=./musica INPUT=./canzoni.csv OUTDIR=./test_output ./test_musica.sh

set -u
set -o pipefail

EXE="${EXE:-./musica}"
INPUT="${INPUT:-./canzoni.csv}"
OUTDIR="${OUTDIR:-./test_output}"
LOGDIR="$OUTDIR/log"

mkdir -p "$OUTDIR" "$LOGDIR"
rm -f "$OUTDIR"/test_*.csv "$OUTDIR"/stdout_*.csv "$LOGDIR"/* 2>/dev/null || true

PASS=0
FAIL=0
OBS=0
last_rc=0
last_stdout=""
last_stderr=""

pass() { PASS=$((PASS+1)); printf '[PASS] %s\n' "$1"; }
fail() { FAIL=$((FAIL+1)); printf '[FAIL] %s\n' "$1"; }
obs()  { OBS=$((OBS+1));   printf '[OSS ] %s\n' "$1"; }

safe_name() {
    printf '%s' "$1" | tr '[:upper:]' '[:lower:]' |
        tr ' /:"' '_____' | tr -cd '[:alnum:]_.-'
}

run_raw() {
    local name="$1"; shift
    local safe
    safe="$(safe_name "$name")"
    last_stdout="$LOGDIR/${safe}.stdout"
    last_stderr="$LOGDIR/${safe}.stderr"

    {
        printf '%q ' "$EXE"
        printf '%q ' "$@"
        printf '\n'
    } > "$LOGDIR/${safe}.command"

    "$EXE" "$@" >"$last_stdout" 2>"$last_stderr"
    last_rc=$?
}

expect_success() {
    local name="$1"; shift
    run_raw "$name" "$@"
    if (( last_rc == 0 )); then pass "$name"; else fail "$name: rc=$last_rc"; fi
}

expect_failure() {
    local name="$1"; shift
    run_raw "$name" "$@"
    if (( last_rc != 0 )); then
        pass "$name: errore intercettato (rc=$last_rc)"
    else
        fail "$name: atteso errore, ottenuto rc=0"
    fi
}

observe_case() {
    local name="$1"; shift
    run_raw "$name" "$@"
    obs "$name: rc=$last_rc; vedere $last_stdout e $last_stderr"
}

expect_lines() {
    local label="$1" file="$2" expected="$3"
    local actual
    [[ -f "$file" ]] || { fail "$label: file non creato"; return; }
    actual="$(wc -l < "$file")"
    [[ "$actual" -eq "$expected" ]] &&
        pass "$label: $actual righe" ||
        fail "$label: attese $expected, trovate $actual"
}

expect_empty() {
    local label="$1" file="$2"
    [[ ! -s "$file" ]] && pass "$label" || fail "$label: $file non vuoto"
}

expect_same() {
    local label="$1" first="$2" second="$3"
    if cmp -s "$first" "$second"; then
        pass "$label"
    else
        fail "$label"
        diff -u "$first" "$second" > "$LOGDIR/$(safe_name "$label").diff" || true
    fi
}

expect_contains() {
    local label="$1" file="$2" text="$3"
    grep -Fqi -- "$text" "$file" && pass "$label" || fail "$label"
}

expect_year_sorted() {
    local label="$1" file="$2"
    if awk -F';' '
        NR==1 { prev=$3+0; next }
        ($3+0)<prev { exit 1 }
        { prev=$3+0 }
        END { if (NR==0) exit 1 }
    ' "$file"; then pass "$label"; else fail "$label"; fi
}

expect_runtime_sorted() {
    local label="$1" file="$2"
    if awk -F';' '
        {
            curr=($4+0)*60+($5+0)
            if (NR>1 && curr<prev) exit 1
            prev=curr
        }
        END { if (NR==0) exit 1 }
    ' "$file"; then pass "$label"; else fail "$label"; fi
}

expect_duration_le() {
    local label="$1" file="$2" minutes="$3"
    awk -F';' -v lim="$((minutes*60))" '
        (($4+0)*60+($5+0))>lim { exit 1 }
    ' "$file" && pass "$label" || fail "$label"
}

expect_all_year() {
    local label="$1" file="$2" year="$3"
    awk -F';' -v y="$year" '($3+0)!=y { exit 1 }' "$file" &&
        pass "$label" || fail "$label"
}

expect_search_or() {
    local label="$1" file="$2" needle="$3"
    awk -F';' -v q="$needle" '
        BEGIN { q=tolower(q) }
        {
            if (index(tolower($1),q)==0 && index(tolower($2),q)==0) exit 1
        }
    ' "$file" && pass "$label" || fail "$label"
}

expect_search_title() {
    local label="$1" file="$2" needle="$3"
    awk -F';' -v q="$needle" '
        BEGIN { q=tolower(q) }
        index(tolower($1),q)==0 { exit 1 }
    ' "$file" && pass "$label" || fail "$label"
}

expect_search_artist() {
    local label="$1" file="$2" needle="$3"
    awk -F';' -v q="$needle" '
        BEGIN { q=tolower(q) }
        index(tolower($2),q)==0 { exit 1 }
    ' "$file" && pass "$label" || fail "$label"
}

run_both() {
    local test_name="$1" outfile="$2"; shift 2
    local stdout_copy="$OUTDIR/stdout_${test_name}.csv"

    expect_success "$test_name senza outfile" "$INPUT" "$@"
    cp "$last_stdout" "$stdout_copy"

    printf 'SPAZZATURA DA TRONCARE\n' > "$outfile"
    expect_success "$test_name con outfile" "$INPUT" "$@" "$outfile"

    expect_empty "$test_name: stdout vuoto con outfile" "$last_stdout"
    expect_same "$test_name: stdout e outfile coincidono" "$stdout_copy" "$outfile"
}

printf 'Eseguibile: %s\nInput: %s\nOutput: %s\n\n' "$EXE" "$INPUT" "$OUTDIR"

[[ -x "$EXE" ]] || { printf 'Eseguibile non trovato: %s\n' "$EXE" >&2; exit 2; }
[[ -f "$INPUT" ]] || { printf 'Input non trovato: %s\n' "$INPUT" >&2; exit 2; }

# A. Help e grammatica
expect_failure "nessun argomento"
observe_case   "help minuscolo" help
observe_case   "help maiuscolo" HELP
expect_failure "help con argomento eccedente" help extra
expect_failure "solo file input" "$INPUT"
expect_failure "comando sconosciuto" "$INPUT" paperino
expect_failure "input inesistente" "$OUTDIR/file_inesistente.csv" ordina
expect_failure "troppi argomenti dopo ordina" "$INPUT" ordina "$OUTDIR/test_ordina_extra.csv" extra

# B. durata <m>
run_both "durata_3" "$OUTDIR/test_durata_3.csv" durata 3
expect_lines "durata 3: nessun brano <=3:00" "$OUTDIR/test_durata_3.csv" 0

run_both "durata_4" "$OUTDIR/test_durata_4.csv" durata 4
expect_lines "durata 4: numero atteso" "$OUTDIR/test_durata_4.csv" 11
expect_duration_le "durata 4: tutti <=240 secondi" "$OUTDIR/test_durata_4.csv" 4
if grep -Fq 'America;Gianna Nannini;1979;4;20' "$OUTDIR/test_durata_4.csv"; then
    fail "durata 4: 4:20 incluso per errore"
else
    pass "durata 4: 4:20 escluso"
fi
expect_contains "durata 4: include 4:00" "$OUTDIR/test_durata_4.csv" \
    'Abbi cura di me;Simone Cristicchi;2019;4;0'

run_both "durata_5" "$OUTDIR/test_durata_5.csv" DURATA 5
expect_lines "durata 5: numero atteso" "$OUTDIR/test_durata_5.csv" 19
expect_duration_le "durata 5: tutti <=300 secondi" "$OUTDIR/test_durata_5.csv" 5

run_both "durata_6" "$OUTDIR/test_durata_6.csv" durata 6
expect_lines "durata 6: tutti i brani" "$OUTDIR/test_durata_6.csv" 25

expect_failure "durata senza operando" "$INPUT" durata
expect_failure "durata operando vuoto" "$INPUT" durata ""
observe_case   "durata zero" "$INPUT" durata 0
observe_case   "durata negativa" "$INPUT" durata -1
observe_case   "durata non numerica" "$INPUT" durata pippo
observe_case   "durata prefisso numerico con suffisso" "$INPUT" durata 4pippo
expect_failure "durata con troppi argomenti" "$INPUT" durata 4 "$OUTDIR/test_durata_4_extra.csv" extra

# C. cerca <stringa>
run_both "cerca_queen" "$OUTDIR/test_cerca_queen.csv" cerca queen
expect_lines "cerca queen: artista Queen + titolo Dancing Queen" "$OUTDIR/test_cerca_queen.csv" 2
expect_search_or "cerca queen: match titolo OR interprete" "$OUTDIR/test_cerca_queen.csv" queen

run_both "cerca_queen_mixed_case" "$OUTDIR/test_cerca_queen_mixed_case.csv" CERCA qUeEn
expect_lines "cerca qUeEn: case-insensitive" "$OUTDIR/test_cerca_queen_mixed_case.csv" 2
expect_same "cerca queen: stesso risultato con case diverso" \
    "$OUTDIR/test_cerca_queen.csv" "$OUTDIR/test_cerca_queen_mixed_case.csv"

run_both "cerca_lucio" "$OUTDIR/test_cerca_lucio.csv" cerca Lucio
expect_lines "cerca Lucio: cinque risultati" "$OUTDIR/test_cerca_lucio.csv" 5
expect_search_or "cerca Lucio: match titolo OR interprete" "$OUTDIR/test_cerca_lucio.csv" lucio

run_both "cerca_zzzz" "$OUTDIR/test_cerca_zzzz.csv" cerca zzzz
expect_lines "cerca zzzz: risultato vuoto" "$OUTDIR/test_cerca_zzzz.csv" 0

expect_failure "cerca senza operando" "$INPUT" cerca
expect_failure "cerca stringa vuota" "$INPUT" cerca ""
observe_case   "cerca soli spazi quotati" "$INPUT" cerca "   "
expect_failure "cerca con troppi argomenti" "$INPUT" cerca queen "$OUTDIR/test_cerca_extra.csv" extra

# D. anno <anno>
run_both "anno_1979" "$OUTDIR/test_anno_1979.csv" anno 1979
expect_lines "anno 1979: cinque risultati" "$OUTDIR/test_anno_1979.csv" 5
expect_all_year "anno 1979: valori corretti" "$OUTDIR/test_anno_1979.csv" 1979

run_both "anno_1970" "$OUTDIR/test_anno_1970.csv" ANNO 1970
expect_lines "anno 1970: due risultati" "$OUTDIR/test_anno_1970.csv" 2
expect_all_year "anno 1970: valori corretti" "$OUTDIR/test_anno_1970.csv" 1970

run_both "anno_1962" "$OUTDIR/test_anno_1962.csv" anno 1962
expect_lines "anno 1962: un risultato" "$OUTDIR/test_anno_1962.csv" 1

run_both "anno_1985" "$OUTDIR/test_anno_1985.csv" anno 1985
expect_lines "anno 1985: risultato vuoto" "$OUTDIR/test_anno_1985.csv" 0

expect_failure "anno senza operando" "$INPUT" anno
expect_failure "anno stringa vuota" "$INPUT" anno ""
expect_failure "anno non numerico" "$INPUT" anno pippo
expect_failure "anno sotto range" "$INPUT" anno 1799
expect_failure "anno sopra range" "$INPUT" anno 3000
expect_failure "anno con suffisso" "$INPUT" anno 1979pippo
expect_failure "anno con troppi argomenti" "$INPUT" anno 1979 "$OUTDIR/test_anno_extra.csv" extra

# E. ordina
run_both "ordina" "$OUTDIR/test_ordina.csv" ordina
expect_lines "ordina: conserva 25 record" "$OUTDIR/test_ordina.csv" 25
expect_year_sorted "ordina: anni non decrescenti" "$OUTDIR/test_ordina.csv"

run_both "ordina_uppercase" "$OUTDIR/test_ordina_uppercase.csv" ORDINA
expect_same "ordina: comando case-insensitive" \
    "$OUTDIR/test_ordina.csv" "$OUTDIR/test_ordina_uppercase.csv"

# F. comandi in sviluppo
run_both "cercai_lucio" "$OUTDIR/test_cercai_lucio.csv" cercai lucio
expect_lines "cercai lucio: cinque interpreti" "$OUTDIR/test_cercai_lucio.csv" 5
expect_search_artist "cercai lucio: solo interprete" "$OUTDIR/test_cercai_lucio.csv" lucio

run_both "cercat_queen" "$OUTDIR/test_cercat_queen.csv" cercat queen
expect_lines "cercat queen: solo Dancing Queen" "$OUTDIR/test_cercat_queen.csv" 1
expect_search_title "cercat queen: solo titolo" "$OUTDIR/test_cercat_queen.csv" queen

run_both "list" "$OUTDIR/test_list.csv" List
expect_lines "List: conserva 25 record" "$OUTDIR/test_list.csv" 25
expect_same "List: identico all'input" "$INPUT" "$OUTDIR/test_list.csv"

run_both "list_alias_l" "$OUTDIR/test_l.csv" L
expect_same "L: alias identico a List" "$OUTDIR/test_list.csv" "$OUTDIR/test_l.csv"

run_both "ordlen" "$OUTDIR/test_ordlen.csv" OrdLen
expect_lines "OrdLen: conserva 25 record" "$OUTDIR/test_ordlen.csv" 25
expect_runtime_sorted "OrdLen: runtime non decrescente" "$OUTDIR/test_ordlen.csv"

run_both "ordlen_alias_ol" "$OUTDIR/test_ol.csv" OL
expect_same "OL: alias identico a OrdLen" "$OUTDIR/test_ordlen.csv" "$OUTDIR/test_ol.csv"

pensiero="$(grep -nF 'Pensiero;Pooh;1971;3;57' "$OUTDIR/test_ordlen.csv" | cut -d: -f1)"
mare="$(grep -nF 'Ci vorrebbe il mare;Marco Masini;1990;3;57' "$OUTDIR/test_ordlen.csv" | cut -d: -f1)"
eye="$(grep -nF 'Eye in the sky;Alan Parson Project;1981;4;25' "$OUTDIR/test_ordlen.csv" | cut -d: -f1)"
migliori="$(grep -nF 'I migliori anni della nostra vita;Renato Zero;1995;4;25' "$OUTDIR/test_ordlen.csv" | cut -d: -f1)"

[[ -n "$pensiero" && -n "$mare" && "$pensiero" -lt "$mare" ]] &&
    pass "OrdLen stabile sul pareggio 3:57" ||
    fail "OrdLen non stabile sul pareggio 3:57"

[[ -n "$eye" && -n "$migliori" && "$eye" -lt "$migliori" ]] &&
    pass "OrdLen stabile sul pareggio 4:25" ||
    fail "OrdLen non stabile sul pareggio 4:25"

printf '\n========================================\n'
printf 'PASS         : %d\n' "$PASS"
printf 'FAIL         : %d\n' "$FAIL"
printf 'OSSERVAZIONI : %d\n' "$OBS"
printf 'CSV prodotti : %s\n' "$OUTDIR"
printf 'Log          : %s\n' "$LOGDIR"
printf '========================================\n'

(( FAIL == 0 ))

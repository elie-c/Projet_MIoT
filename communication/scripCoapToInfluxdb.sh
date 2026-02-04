#!/usr/bin/env bash
set -euo pipefail

##### Constantes facilement modifiables #####
INTERVAL_SEC=5

COAP_HOST="[aaaa::212:4b00:18ec:28a7]"
COAP_PORT=""                    # ex ":5683"
COAP_BASE="coap://${COAP_HOST}${COAP_PORT}"

INFLUX_URL="http://miot.istic.univ-rennes.fr:8086"
INFLUX_ORG="de63a4b387f75b50"
INFLUX_BUCKET="miot"
INFLUX_TOKEN="jbqh_NdXS7XLjviaJKq8HHoQGXyvPDRyDaiTKb0wwuYQphvYk8cmDa2McynWcfo_IUyY8QsjiIoYeHImzLD__A=="

INFLUX_MEAS="coap_metrics"
INFLUX_TAGS="node=aaaa_212_4b00_18ec_28a7"

VERBOSE=1            # 1=logs, 0=silencieux
FAIL_ON_WRITE=0      # 1=stop si écriture Influx échoue, 0=continue
############################################

log() { [[ "${VERBOSE}" == "1" ]] && echo "[$(date -Is)] $*" >&2 || true; }

need_cmd() { command -v "$1" >/dev/null 2>&1 || { echo "Manque la commande: $1" >&2; exit 1; }; }
need_cmd coap-client
need_cmd curl
need_cmd sed
need_cmd date

coap_get_json() {
  local path="$1"
  # On récupère stdout (payload). On log aussi stderr si utile.
  local out err rc
  err="$(mktemp)"
  set +e
  out="$(coap-client -m get "${COAP_BASE}${path}" 2>"$err")"
  rc=$?
  set -e
  if [[ $rc -ne 0 ]]; then
    log "COAP ERROR rc=${rc} path=${path} stderr=$(tr '\n' ' ' <"$err")"
    rm -f "$err"
    echo ""
    return 0
  fi
  if [[ -s "$err" ]]; then
    log "COAP stderr path=${path}: $(tr '\n' ' ' <"$err")"
  fi
  rm -f "$err"
  echo "$out"
}

json_get_num() {
  local key="$1"
  sed -nE "s/.*\"${key}\"[[:space:]]*:[[:space:]]*([-]?[0-9]+([.][0-9]+)?).*/\1/p"
}

influx_write_lp_verbose() {
  local lp="$1"
  local write_url="${INFLUX_URL}/api/v2/write?org=${INFLUX_ORG}&bucket=${INFLUX_BUCKET}&precision=ns"

  log "Influx POST ${write_url}"
  log "Line protocol => ${lp}"

  # On récupère code HTTP + réponse (utile si 401/403/400)
  local body_file http_code curl_rc
  body_file="$(mktemp)"
  set +e
  http_code="$(curl -sS \
    -o "${body_file}" \
    -w "%{http_code}" \
    --request POST \
    "${write_url}" \
    --header "Authorization: Token ${INFLUX_TOKEN}" \
    --header "Content-Type: text/plain; charset=utf-8" \
    --header "Accept: application/json" \
    --data-binary "${lp}")"
  curl_rc=$?
  set -e

  if [[ $curl_rc -ne 0 ]]; then
    log "Influx CURL ERROR rc=${curl_rc}"
    log "Influx BODY: $(cat "${body_file}")"
    rm -f "${body_file}"
    return 1
  fi

  if [[ "${http_code}" != "204" ]]; then
    log "Influx HTTP=${http_code} (attendu 204) [page:1]"
    log "Influx BODY: $(cat "${body_file}")"
    rm -f "${body_file}"
    return 2
  fi

  log "Influx OK HTTP=204 (write accepted) [page:1]"
  rm -f "${body_file}"
  return 0
}

log "Starting: interval=${INTERVAL_SEC}s coap_base=${COAP_BASE} influx=${INFLUX_URL} org=${INFLUX_ORG} bucket=${INFLUX_BUCKET} [page:1]"

while true; do
  ts_ns="$(date +%s%N)"
  log "---- cycle ts_ns=${ts_ns} ----"

  u_json="$(coap_get_json "/u")"
  c_json="$(coap_get_json "/c")"
  soc_json="$(coap_get_json "/soc")"

  log "CoAP /u   => ${u_json}"
  log "CoAP /c   => ${c_json}"
  log "CoAP /soc => ${soc_json}"

  uPan="$(printf '%s' "$u_json"  | json_get_num "uPan" || true)"
  uBat="$(printf '%s' "$u_json"  | json_get_num "uBat" || true)"
  cPan="$(printf '%s' "$c_json"  | json_get_num "cPan" || true)"
  cBat="$(printf '%s' "$c_json"  | json_get_num "cBat" || true)"
  soc="$(printf '%s' "$soc_json" | json_get_num "soc"  || true)"

  log "Parsed: uPan='${uPan}' uBat='${uBat}' cPan='${cPan}' cBat='${cBat}' soc='${soc}'"

  if [[ -z "${uPan}" || -z "${uBat}" || -z "${cPan}" || -z "${cBat}" || -z "${soc}" ]]; then
    log "Parse KO -> skip write"
    sleep "${INTERVAL_SEC}"
    continue
  fi

  lp="${INFLUX_MEAS},${INFLUX_TAGS} uPan=${uPan}i,uBat=${uBat}i,cPan=${cPan}i,cBat=${cBat}i,soc=${soc}i ${ts_ns}"

  set +e
  influx_write_lp_verbose "${lp}"
  wrc=$?
  set -e

  if [[ $wrc -ne 0 ]]; then
    log "Write FAILED rc=${wrc}"
    if [[ "${FAIL_ON_WRITE}" == "1" ]]; then
      exit 1
    fi
  fi

  sleep "${INTERVAL_SEC}"
done

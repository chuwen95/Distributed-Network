listen_ip="0.0.0.0"
listen_port_start=(30300 20200 8500)
cluster_all_servers=["node0","node1","node2","node3"]
output_dir="./nodes"
ip_param=
binary_name="CopyStateMachine"
binary_filename=

log_info()
{
    echo -e "\033[32m[INFO] $1\033[0m"
}

log_warn()
{
    echo -e "\033[31m[WARN] $1\033[0m"
}

log_error()
{
    echo -e "\033[31m[ERROR] $1\033[0m"
}

is_dir_exist()
{
    if [ -d "$1" ]; then
        return 0
    fi

    return 1
}

is_file_exist()
{
    if [ -f "$1" ]; then
        return 0
    fi

    return 1
}

is_ip_legal()
{
    if ! [ -z $(echo $1 | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$") ]; then
        return 0
    fi

    return 1
}

is_value_num()
{
    if [[ "$1" =~ ^-?[0-9]+$ ]]; then
        return 0
    fi

    return 1
}

help()
{
    echo $1
    cat  << EOF
Usage:
-h Help
-l <IP list>               [Required] "ip1:nodeNum1, ip2:nodeNum2" e.g:"192.168.0.1:2,192.168.0.2:3"
-p <Start port>            [Option] Default 30300,20200,8500 means p2p_port start from 30300, tcp_rpc_port from 20200, http_tpc_port from 8500
-o <output dir>            [Option] output directoy, default ./nodes
EOF

    exit 0
}

parse_params()
{
    while getopts "h:l:p:o:e:" option; do
        case $option in
        h)
            help
            ;;
        l)
            ip_param=$OPTARG
            ;;
        p)
            listen_port_start=(${OPTARG//,/})
            if [ ${#listen_port_start[@]} -ne 3 ]; then
                log_error "p2p start port error, e.g: 30300"
                exit 1
            fi
            ;;
        o)
            output_dir=$OPTARG
            ;;
        e)
            binary_filename=$OPTARG
            ;;
        *)
            log_error "Invalid option: -$option"
            exit 1
            ;;
        esac
    done
}

generate_all_node_start_stop_scripts()
{
    local output_dir=${1}
    mkdir -p ${output_dir}

    cat <<EOF >"${output_dir}/start_all.sh"
#!/bin/bash
cur_script_path="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${cur_script_path}"

sub_dirs_of_script_path=(\$(ls -l \${cur_script_path} | awk '/^d/ {print \$NF}'))
for dir in \${sub_dirs_of_script_path[*]}
do
    if [[ -f "\${cur_script_path}/\${dir}/config.ini" && -f "\${cur_script_path}/\${dir}/start.sh" ]]; then
        echo "try to start \${dir}"
        bash \${cur_script_path}/\${dir}/start.sh &
    fi
done

wait
EOF
    chmod u+x "${output_dir}/start_all.sh"

    cat <<EOF >"${output_dir}/stop_all.sh"
#!/bin/bash
cur_script_path="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\${cur_script_path}"

sub_dirs_of_script_path=(\$(ls -l \${cur_script_path} | awk '/^d/ {print \$NF}'))
for dir in \${sub_dirs_of_script_path[*]}
do
    if [[ -f "\${cur_script_path}/\${dir}/config.ini" && -f "\${cur_script_path}/\${dir}/stop.sh" ]]; then
        echo "try to stop \${dir}"
        bash \${cur_script_path}/\${dir}/stop.sh &
    fi
done

wait
EOF
    chmod u+x "${output_dir}/stop_all.sh"
}

generate_node_ini_config()
{
    local node_config_path="${1}"
    local node_id="${2}"
    local p2p_listen_ip="${3}"
    local p2p_listen_port="${4}"
    local tcp_rpc_listen_ip="${5}"
    local tcp_rpc_listen_port="${6}"
    local http_rpc_listen_ip="${7}"
    local http_rpc_listen_port="${8}"

    cat <<EOF > "${node_config_path}"
[cluster]
id=$node_id
servers=$cluster_all_servers

[rpc]
http_listen_ip=${http_rpc_listen_ip}
http_listen_port=${http_rpc_listen_port}
tcp_listen_ip=${tcp_rpc_listen_ip}
tcp_listen_port=${tcp_rpc_listen_port}

[network]
ip=${p2p_listen_ip}
port=${p2p_listen_port}
nodes_file=nodes.json
packet_process_worker_num=8

[reactor]
slave_reactor_num=8
redispatch_interval=50

[logger]
enable_file_log=true
console_output=false
; trace/debug/info/warning/error
level=info
path=./logs
EOF
}

generate_node_p2p_connect_node_json()
{
    local -n nodes_dir_array_ref=$1
    local -n connect_nodes_array_ref=$2

    local connect_nodes_json="{\"nodes\": ["

    connect_nodes_length=${#connect_nodes_array_ref[@]}
    for ((i = 0; i < connect_nodes_length; ++i)); do
        if [ $((i + 1)) -ne $connect_nodes_length ]; then
            connect_nodes_json="${connect_nodes_json}\"${connect_nodes_array_ref[$i]}\", "
        else
            connect_nodes_json="${connect_nodes_json}\"${connect_nodes_array_ref[$i]}\"]}"
        fi
    done

    for node_dir in "${nodes_dir_array_ref[@]}"; do
        cat <<EOF >"${node_dir}/nodes.json"
$connect_nodes_json
EOF
    done
}

generate_scripts_common_contents()
{
    local filepath=$1
    local dirpath=$(dirname $filepath)
    cat << EOF > $filepath
#!/bin/bash

log_info()
{
    echo -e "\033[32m[INFO] \$1\033[0m"
}

log_warn()
{
    echo -e "\033[31m[WARN] \$1\033[0m"
}

log_error()
{
    echo -e "\033[31m[ERROR] \$1\033[0m"
}

EOF
}

generate_node_start_stop_scripts()
{
    local output=$1
    local ps_cmd="ps aux | grep \$dis_net_node | grep -v grep | awk '{print \$2}'"
    local start_cmd="nohup \$dis_net_node config.ini > nohup.out 2>&1 &"
    local stop_cmd="kill \$node_pid"
    local log_cmd="tail -n20 \$shell_folder/nohup.out"
    local find_start_success_in_std_output_cmd="$log_cmd | grep \"Initializer start successfully\""

    generate_scripts_common_contents "$output/start.sh"
    cat << EOF >> "$output/start.sh"
shell_folder=\$(cd \$(dirname \$0);pwd)
dis_net_node=\$shell_folder/../${binary_name}

cd \$shell_folder

node_dir_name=\$(basename \$shell_folder)
node_pid=\$($ps_cmd)
if [ ! -z \$node_pid ]; then
    log_info "\$node_dir_name is running, pid is \$node_pid."
    exit 0
else
    ${start_cmd}
    sleep 1.5
fi

success_flag=false
try_times=4
cur_try_time=0
while [ \$cur_try_time -lt \$try_times ]
do
    node_pid=\$($ps_cmd)
    is_start_success_in_std_output=\$($find_start_success_in_std_output_cmd)
    if [[ ! -z \$node_pid && ! -z "\$is_start_success_in_std_output" ]];then
        success_flag=true
        break
    fi
    sleep 0.5
    ((cur_try_time=cur_try_time+1))
done

if [ \$success_flag = true ];then
    echo -e "\033[32m \$node_dir_name start successfully pid=\$node_pid\033[0m"
else
    echo -e "\033[31m  Exceed waiting time. Please try again to start \${node} \033[0m"
fi

EOF
    chmod u+x "$output/start.sh"

    generate_scripts_common_contents "$output/stop.sh"
    cat << EOF >> "$output/stop.sh"
shell_folder=\$(cd \$(dirname \$0);pwd)
dis_net_node=\$shell_folder/../\$binary_name
node_dir_name=\$(basename \$shell_folder)
node_pid=\$($ps_cmd)

if [ -z \$node_pid ]; then
    echo "\$node_dir_name isn't running."
    exit 0
fi

[ ! -z \$node_pid ] && $stop_cmd > /dev/null

success_flag=false
try_times=10
cur_try_time=0
while [ \$cur_try_time -lt \$try_times ]
do
    sleep 1
    node_pid=\$($ps_cmd)
    if [ -z \$node_pid ]; then
        success_flag=true
        break
    fi
    ((cur_try_time=cur_try_time+1))
done

if [ "\$success_flag" = true ];then
    echo -e "\033[32m stop \$node_dir_name success.\033[0m"
else
    echo "  Exceed maximum number of retries. Please try again to stop \${node}"
fi
EOF
    chmod u+x "$output/stop.sh"
}

generate_nodes()
{
    [ -z "$ip_param" ] && help 'ERROR: Please set -l option.'

    # 目标文件夹必须不存在
    if is_dir_exist "${output_dir}"; then
        log_error "${output_dir} must not exist"
        exit 1
    fi

    mkdir -p "$output_dir"

    # 检查目标文件夹是否创建成功
    if ! is_dir_exist "${output_dir}"; then
        log_error "${output_dir} create failed"
        exit 1
    fi

    local p2p_start_port=listen_port_start[0]
    local tcp_rpc_start_port=listen_port_start[1]
    local tcp_http_start_port=listen_port_start[2]

    # 根据逗号分割ip_param，并将结果存储到ip_param_split_array数组
    IPS=',' read -ra ip_param_split_array <<< "$ip_param"

    # 用来生成connect nodes数据的变量
    local node_dir_array=()
    local connect_nodes_array=()
    # 生成所有节点文件夹及内容
    for ip_param in ${ip_param_split_array[*]}; do
        ip=${ip_param%:*}
        num=${ip_param#*:}

        # 检查ip是否合法
        if ! is_ip_legal ${ip}; then
            log_warn "Please check ip address: ${ip}, if you use domain name please ignore this."
        fi

        # 检查num是否是数字
        if ! is_value_num ${num}; then
            log_error "the num of ip error, it must be digital: ${ip_param}"
            exit 1
        fi

        nodes_dir="${output_dir}/${ip}"
        # 生成start_all.sh和stop_all.sh
        generate_all_node_start_stop_scripts "$nodes_dir"
        cp $binary_filename $nodes_dir
        # 生成每个节点
        for ((i = 0; i < num; ++i)); do
            local node_index=${i}
            # 节点名称
            local node_dir="${output_dir}/${ip}/node${node_index}"
            # 创建节点文件夹
            mkdir -p ${node_dir}

            node_dir_array+=("$node_dir")

            # 生成节点config.ini配置文件
            local node_ini_config_file_name="${node_dir}/config.ini"
            local node_id="node${node_index}"
            local p2p_listen_port=$((p2p_start_port + node_index))
            local tcp_rpc_listen_port=$((tcp_rpc_start_port + node_index))
            local http_rpc_listen_port=$((http_rpc_start_port + node_index))
            generate_node_ini_config "${node_ini_config_file_name}" "${node_id}" "${listen_ip}" "${p2p_listen_port}" "${listen_ip}" ${tcp_rpc_listen_port} "127.0.0.1" "${http_rpc_listen_port}"
            generate_node_start_stop_scripts $node_dir

            # 创建所有要连接的节点数据
            connect_nodes_array+=("${ip}:${p2p_listen_port}")
        done
    done

    generate_node_p2p_connect_node_json node_dir_array connect_nodes_array
}

main()
{
    parse_params "$@"
    generate_nodes
}

main "$@"
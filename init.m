% 电机参数
np = 7;
rs_ohm_real = 5.1;
ld_h_real = 2.8e-3;
lq_h_real = 2.8e-3;
flux_linkage_wb_real = 1.79e-3;
vdc_v_real = 12.0;
i_max_a_real = 1; %最大量程的电流(留一定余量)
rpm = 2700;%电机转速
fn = rpm/60*np;%电频率

switch_freq_hz_real = 8e3;%开关频率
ts = 1/switch_freq_hz_real;%采样周期
current_wc_real = (switch_freq_hz_real/10)*2*pi;%电流环设计带宽


% 基准量选择电压、电流、电角速度
v_base = vdc_v_real/sqrt(3);%v的基值一定要选这个，可以在计算svpwm的时候抵消掉k
i_base = i_max_a_real;%A
inv_v_base = 1/v_base;
inv_i_base = 1/i_base;
omega_base = 2*pi*fn;%rad/s
ts_base = 1/omega_base;%s

% 电阻、电感通过推导获得
r_base = v_base/i_base;
l_base = r_base/(omega_base);
phi_base = v_base/omega_base;

% 电机参数归一化
r_pu = rs_ohm_real/r_base;
ld_pu = ld_h_real/l_base;
lq_pu = lq_h_real/l_base;
phi_pu = flux_linkage_wb_real/phi_base;

current_wc_pu = current_wc_real/omega_base;
ts_pu = ts/ts_base;
id_kp_pu = ld_pu * current_wc_pu;
id_ki_pu = r_pu * current_wc_pu * ts_pu;
iq_kp_pu = lq_pu * current_wc_pu;
iq_ki_pu = r_pu * current_wc_pu * ts_pu;

% 零飘在线滤波器(停机零偏估计)参数设计
zero_drift_filter_fs = switch_freq_hz_real; % Hz, 与电流采样频率一致
zero_drift_filter_fc = 10;                  % Hz, 一阶低通截止频率
zero_drift_filter_order = 1;
[zero_drift_b, zero_drift_a] = butter(zero_drift_filter_order, ...
    zero_drift_filter_fc/(zero_drift_filter_fs/2), ...
    'low');

fprintf('\n零飘一阶低通滤波器(浮点)系数:\n');
fprintf('b = [%.8f, %.8f]\n', zero_drift_b(1), zero_drift_b(2));
fprintf('a = [%.8f, %.8f]\n', zero_drift_a(1), zero_drift_a(2));

fprintf('\n可直接填写到 C 宏:\n');
fprintf('#define FOC_CURRENT_ZERO_DRIFT_FILTER_B0 %.8ff\n', zero_drift_b(1));
fprintf('#define FOC_CURRENT_ZERO_DRIFT_FILTER_B1 %.8ff\n', zero_drift_b(2));
fprintf('#define FOC_CURRENT_ZERO_DRIFT_FILTER_A1 %.8ff\n', zero_drift_a(2));

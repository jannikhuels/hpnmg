<?php

define('LF', "\n");

define('SECONDS', 1);
define('MINUTES', 60 * SECONDS);
define('HOURS', 60 * MINUTES);
define('WATTSECONDS', 1);
define('WATTMINUTES', WATTSECONDS * MINUTES);
define('WATTHOURS', WATTSECONDS * HOURS);
define('KILOWATTSECONDS', 1000 * WATTSECONDS);
define('KILOWATTMINUTES', 1000 * WATTMINUTES);
define('KILOWATTHOURS', 1000 * WATTHOURS);

define('TIME_PER_UNIT', 1 * SECONDS);

define('SAMPLE_POINTS', 25);

if (SAMPLE_POINTS < 2)
  die('please choose at least 2 sample points' . LF);

if ($argc != 4 || !in_array($argv[1], ['formula', 'encoded']))
    die('please pass "formula" or "encoded" pps and ppd' . LF);

$regular_formula = $argv[1] === 'formula';
$pps = $argv[2];
$ppd = $argv[3];

$model_tpl = file_get_contents($regular_formula ? 'ev_charging.xml' : 'ev_charging_robustness.xml') or die('no model' . LF);
$formula_tpl = file_get_contents($regular_formula ? 'ev_formula.xml' : 'ev_formula_robustness.xml') or die('no formula' . LF);

$max_time = 24 * HOURS;
$mu_return = 8 * HOURS; // average customer return time
$mean_recovery = 1 * HOURS;
$mu_demand = 1 * HOURS;
$sigma_demand = 10 * MINUTES;
$t_pr = 90 * MINUTES; // time to price recovery
$t_di = 4 * HOURS; // time to demand increase
$max_soc = 90 * KILOWATTHOURS;
$r_base = $max_soc / ($mu_return / 2); // base rate fully charges in half the expected return time
$r_g2v = 120 * KILOWATTHOURS / HOURS - $r_base;

$lambda_price_slump = $pps / (24 * HOURS);
$mu_decrease = 90 * MINUTES;
$sigma_decrease = 20 * MINUTES;

[$model_tpl, $formula_tpl] = str_replace(
    [
        '%max_time%',
        '%mu_return%',
        '%mean_recovery%',
        '%mu_demand%',
        '%sigma_demand%',
        '%t_pr%',
        '%t_di%',
        '%max_soc%',
        '%r_base%',
        '%r_g2v%',
        '%lambda_price_slump%',
        '%mu_decrease%',
        '%sigma_decrease%',
    ],
    [
        $max_time / TIME_PER_UNIT,
        $mu_return / TIME_PER_UNIT,
        $mean_recovery / TIME_PER_UNIT,
        $mu_demand / TIME_PER_UNIT,
        $sigma_demand / TIME_PER_UNIT,
        $t_pr / TIME_PER_UNIT,
        $t_di / TIME_PER_UNIT,
        ($max_soc / TIME_PER_UNIT) + 1, //TODO: little hack to allow "place is 100% full"
        $r_base,
        $r_g2v,
        $lambda_price_slump / TIME_PER_UNIT,
        $mu_decrease / TIME_PER_UNIT,
        $sigma_decrease / TIME_PER_UNIT,
    ],
    [$model_tpl, $formula_tpl]
);

$m_ps = [[0,0], [1,1], [2,2]];
$m_ps = [[$pps, $ppd]];
$esocs = [0.5 * $max_soc, 0.75 * $max_soc, 0.95 * $max_soc];
$sigma_returns = [7.5 * MINUTES, 60 * MINUTES];
$sigma_returns = [7.5 * MINUTES];
$r_v2gs = [2 * $r_base, 2 * $r_base + $r_g2v, 2 * $r_base + 2 * $r_g2v];

echo 'I will create tons of files in ' . getcwd() . '. Enter to continue.' . LF;
fgets(STDIN);
echo 'Enter once again, just to be sure' . LF;
fgets(STDIN);
echo 'Okay. Let\'s go.' . LF;

foreach ($m_ps as $m_p_i => [$m_pps, $m_ppd])
{
    foreach ($esocs as $esoc_i => $esoc)
    {
        foreach ($sigma_returns as $sigma__return_i => $sigma_return)
        {
            foreach ($r_v2gs as $r_v2g_i => $r_v2g)
            {
//                 for ($i = 0; $i < SAMPLE_POINTS; ++$i) {
//                     $checktime = $mu_return - (4 * $sigma_return) + $i * (8 * $sigma_return) / TIME_PER_UNIT / (SAMPLE_POINTS - 1);

                    $settings = [
                        $m_pps,
                        $m_ppd,
                        $esoc / TIME_PER_UNIT,
                        $sigma_return / TIME_PER_UNIT,
                        $r_v2g,
                    ];
                    [$model, $formula] = str_replace([
                        '%m_pps%',
                        '%m_ppd%',
                        '%esoc%',
                        '%sigma_return%',
                        '%r_v2g%',
                        '%until_bound%',
                    ], array_merge($settings, [$max_time / TIME_PER_UNIT]), [$model_tpl, $formula_tpl]);

                    $settings_string = implode('_', $settings);

//                    $model_file = tempnam(getcwd(), 'model' . ($regular_formula ? '' : '-encoded-robustness') . '_' . $settings_string);
//                    $formula_file = tempnam(getcwd(), 'formula' . ($regular_formula ? '' : '-encoded-robustness') . '_' . $settings_string);
                    $model_file = getcwd() . '/' . 'model' . ($regular_formula ? '' : '-encoded-robustness') . '_' . $settings_string;
                    $formula_file = getcwd() . '/' . 'formula' . ($regular_formula ? '' : '-encoded-robustness') . '_' . $settings_string;
                    file_put_contents($model_file, $model);
                    file_put_contents($formula_file, $formula);

//                    passthru('./main --model ' . $model_file
//                        . ' --formula ' . $formula_file
//                        . ' --maxtime ' . ($max_time / TIME_PER_UNIT)
//                        . ' --checktime ' . ($regular_formula ? 0 : ($max_time / TIME_PER_UNIT))
//                        . ' --result result' . ($regular_formula ? '' : '-encoded-robustness') . '-pps-' . ($pps). '-ppd-' . ($ppd) . '.txt'
//                    );
//                 }
            }
        }
    }
}

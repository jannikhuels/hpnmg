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

if ($argc != 3)
    die('please pass min and max token count for ppd' . LF);

$model_tpl = file_get_contents('ev_charging.xml') or die('no model' . LF);
// $formula_tpl = file_get_contents('ev_formula.xml') or die('no formula' . LF);
$formula_tpl = file_get_contents('return_formula.xml') or die('no formula' . LF);

$max_time = 24 * HOURS;
$mu_return = 8 * HOURS; // average customer return time
$t_pr = 90 * MINUTES; // time to price recovery
$t_di = 4 * HOURS; // time to demand increase
$max_soc = 90 * KILOWATTHOURS;
$r_base = $max_soc / ($mu_return / 2); // base rate fully charges in half the expected return time
$r_g2v = 120 * KILOWATTHOURS / HOURS - $r_base;

[$model_tpl, $formula_tpl] = str_replace(
    [
        '%max_time%',
        '%mu_return%',
        '%t_pr%',
        '%t_di%',
        '%max_soc%',
        '%r_base%',
        '%r_g2v%',
    ],
    [
        $max_time / TIME_PER_UNIT,
        $mu_return / TIME_PER_UNIT,
        $t_pr / TIME_PER_UNIT,
        $t_di / TIME_PER_UNIT,
        $max_soc + 1, //TODO: little hack to allow "place is 100% full"
        $r_base / TIME_PER_UNIT,
        $r_g2v / TIME_PER_UNIT,
    ],
    [$model_tpl, $formula_tpl]
);

$m_ps = [[0,0], [1,1], [2,2]];
    $m_ps = [];
foreach (range($argv[1], $argv[2]) as $tokens)
    $m_ps []= [0, $tokens];

$esocs = [0.9 * $max_soc, 0.95 * $max_soc, 1 * $max_soc];
$esocs = [0.1 * $max_soc];
$sigma_returns = [7.5 * MINUTES, 60 * MINUTES];
$sigma_returns = [7.5 * MINUTES];
$r_v2gs = [2 * $r_base, 2 * $r_base + $r_g2v, 2 * $r_base + 2 * $r_g2v];
$r_v2gs = [1];

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
                $settings = [
                    $m_pps,
                    $m_ppd,
                    $esoc / TIME_PER_UNIT,
                    $sigma_return / TIME_PER_UNIT,
                    $r_v2g / TIME_PER_UNIT,
                ];
                [$model, $formula] = str_replace([
                    '%m_pps%',
                    '%m_ppd%',
                    '%esoc%',
                    '%sigma_return%',
                    '%r_v2g%',
                ], $settings, [$model_tpl, $formula_tpl]);

                $settings_string = implode('_', $settings);

                $model_file = tempnam(getcwd(), 'model_' . $settings_string);
                $formula_file = tempnam(getcwd(), 'formula_' . $settings_string);
                file_put_contents($model_file, $model);
                file_put_contents($formula_file, $formula);

                for ($i = 0; $i < SAMPLE_POINTS; ++$i) {
                    $checktime = $mu_return - (4 * $sigma_return) + $i * (8 * $sigma_return) / TIME_PER_UNIT / (SAMPLE_POINTS - 1);
                    passthru('./main --model ' . $model_file
                        . ' --formula ' . $formula_file
                        . ' --maxtime ' . ($max_time / TIME_PER_UNIT)
                        . ' --checktime ' . $checktime
                        . ' --result return_probs_multidim_' . (2 + $argv[1]). '_to_' . (2 + $argv[2]) . '.txt'
                        . ' --additional ' . $checktime
                    );
                }
            }
        }
    }
}

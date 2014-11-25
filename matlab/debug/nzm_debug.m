%% Parameters
gamma = 0;
n = 2048;
rho = 0.44;
alpha = 0.72;
delta = 1e-8;

fprintf(' - Parameters are: N = %d, \\rho = %.2f, \\alpha = %.2f, \\Delta = %.2e, \\gamma = %d.\n', ...
    n, rho, alpha, delta, gamma)

k = ceil(rho * n);
m = ceil(alpha * n);

%% Generate problem
x = zeros(n, 1);
supp = randperm(n, k);
x(supp) = randn(k, 1);
F = gamma / n + randn(m, n) / sqrt(n);
w = sqrt(delta) * randn(m, 1);
y = F * x + w;

%% Setup algorithm
% Obs.: the 'signal' option is only being passed so that the MSE may be
% evaluated at each iteration; commenting it out won't change the final
% estimate!
outfile = tempname;

opts.solver = 'amp';
opts.delta = 1.0;
opts.learnDelta = 1;
opts.priorDistr = 'gb';
opts.priorPrmts = [rho, 0.0, 1.0];
opts.learnPrior = 0;
opts.initState = [zeros(n, 1); ones(n, 1)];
opts.maxIter = 200;
opts.prec = 1e-8;
opts.display = 1;
opts.signal = x;
opts.output = outfile;

% Extra Feature options
opts.mean_removal = 1;
opts.adaptive_damp = 1;
opts.calc_vfe = 1;
opts.no_violations = 1;

%% Run algorithms
fprintf(' - Running SwAMP... ')
tic
a_sw = swamp(y, F, opts);
elapsed = toc;

out = dlmread(outfile, ';', 1, 0);
mse_sw = out(:, 2);
delta_sw = out(:,3);
rss_sw = out(:,4);
cnv_sw = out(:,5);
fprintf('Elapsed time: %.2fs, MSE: %.2e.\n', elapsed, mse_sw(end)); 

%% Plot results
figure(1); clf;
    subplot(2, 1, 1);
        hold('on');
        plot(x, 'ko'); plot(a_sw, 'rx'); 
        hold('off');
        xlim([0, n]); xlabel('i'); ylabel('x(i)');
        legend('signal', 'swAMP estimate');
        box on;

    subplot(2, 1, 2);
        semilogy(mse_sw);
        xlabel('SwAMP iter.'); ylabel('MSE');
        box on;

figure(2); clf;
    hold on;
        plot(mse_sw,'-b',   'LineWidth',1,'DisplayName','MSE');
        plot(delta_sw,'-m^','LineWidth',1,'DisplayName','\Delta Estimate');
        plot(rss_sw,'-r',   'LineWidth',1,'DisplayName','RSS (residual norm)');
        plot(cnv_sw,'-g',   'LineWidth',1,'DisplayName','Convergence');
        plot(delta*ones(size(delta_sw)),':k','LineWidth',1,'DisplayName','True \Delta');
    hold off;
    xlabel('Iteration');
    set(gca,'YScale','log');    
    box on;
    axis tight;
    legend('Location','NorthEast');


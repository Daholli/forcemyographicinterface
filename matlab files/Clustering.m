clc
close all
clear all

% set how many clusters/gestures you want to differentiate and load the
% data from the drive
clustercount = 3;
load("FSR_data" + ".mat", "FSR_data");

% set the number of principal components
principal_components = 2;
data = FSR_data(all(FSR_data, principal_components),:);

% use the PCA algorith provided by matlab, the outputs are the coefficient
% matrix, the scores for every entry, the explaine variance (how much each
% component attributes to the whole) and the mean to shift the data to
% the origin
[coeff, score, ~, ~, explainedVar, mu] = pca(data);
data_PC = score(:,1:principal_components);

% kmeans which returns the cluster number for each entry of the PCA_data
% and the final centroids
[clusters, centroid] = kmeans(data_PC,clustercount);

colormap_plots = ["#ef476f","#ffd166","#06d6a0","#118ab2","#073b4c"];

% begin plotting

% fig = figure(1);
% f.WindowState = 'maximized';
% title("Trace of FSR-Sensors")
% legend("Location","northeastoutside")
% xlabel("measurement step")
% ylabel("voltage in 8bit format")
% 
% for i=1:8
%     hold("on")
%     plot(data(:,i))
%     hold("off")
% end




f = figure(1);
f.WindowState = 'maximized';

tiledlayout(10,1);
ax1 = nexttile([7,1]);
xlim([-100 100])
ylim([-100 100])
ax2 = nexttile([3,1]);
xlabel("time in computation cycles")
ylabel("Voltage representation 255 equals 3.3V")

title(ax1, "Scatterplot of Data after PCA")
title(ax2, "Data of the eight sensors during training time")




for i = 1:clustercount
    mask = clusters == i;
    idx = find(mask);
    s = repmat([10], numel(idx), 1);

    MarkerColor = colormap_plots(i);

    hold(ax1,"on")
    scatter(ax1,data_PC(idx,1), data_PC(idx,2),s, 'filled', "MarkerFaceColor", MarkerColor);
    hold(ax1,"off")

    hold(ax2,"on")
    scatter(ax2, idx,data(idx,:), s, "filled", "MarkerFaceColor", MarkerColor);
    hold(ax2,"off")
end

figure(2)
bar(explainedVar);
title("Explained Variance of the PCA");
xlabel("principal components");
ylabel("Information in %");
% 
% 
% figure(3)
% tiledlayout(4,1);
% ax3 = nexttile([2,1]);
% ax4 = nexttile([2,1]);
% 
% start_test = 8000;
% end_test = min([start_test + 1000, length(data_PC)]);
% 
% xlim([-60 60])
% ylim([-60 60])
% linkaxes([ax3,ax4], 'xy')
% 
% for i=1:clustercount
%     mask = clusters == i;
%     idx = find(mask);
%     s = repmat([10], numel(idx), 1);
% 
%     MarkerColor = colormap_plots(i);
% 
%     hold(ax3,"on")
%     plot(ax3, data_PC(start_test:end_test,1), data_PC(start_test:end_test,2));
%     hold(ax3,"off")
% 
%     hold(ax4,"on")
%     scatter(ax4,data_PC(idx,1), data_PC(idx,2),s, 'filled', "MarkerFaceColor", MarkerColor);
%     hold(ax4,"off")
% 
% end
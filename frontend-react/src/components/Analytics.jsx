import { TrendingUp, Award } from 'lucide-react';
import './Analytics.css';

function Analytics({ data }) {
  if (!data) return null;

  return (
    <aside className="analytics-panel">
      <h3>
        <TrendingUp size={20} />
        Analytics Dashboard
      </h3>
      
      {data.floors.map((floor, index) => (
        <div key={index} className="analytics-card">
          <h4>{floor.name}</h4>
          
          <div className="analytics-stats">
            <div className="stat">
              <span>Average Usage</span>
              <strong>{floor.avg_usage.toFixed(1)}×</strong>
            </div>
            <div className="stat">
              <span>Peak Usage</span>
              <strong>{floor.max_usage}×</strong>
            </div>
          </div>

          <div className="top-slots">
            <h5>
              <Award size={16} />
              Top 5 Most Used
            </h5>
            <ul>
              {floor.heatmap
                .sort((a, b) => b.usage_count - a.usage_count)
                .slice(0, 5)
                .map((slot, i) => (
                  <li key={i}>
                    <span>{slot.slot_id}</span>
                    <span className="usage-badge">{slot.usage_count}×</span>
                  </li>
                ))}
            </ul>
          </div>
        </div>
      ))}
    </aside>
  );
}

export default Analytics;

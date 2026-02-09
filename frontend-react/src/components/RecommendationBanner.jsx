import { Navigation, X, Check } from 'lucide-react';
import './RecommendationBanner.css';

const RecommendationBanner = ({ slot, onAccept, onDismiss }) => {
  if (!slot) return null;

  return (
    <div className="recommendation-banner">
      <div className="banner-content">
        <Navigation className="banner-icon" />
        <div className="banner-text">
          <h3>🎯 Recommended Slot</h3>
          <p>
            We recommend <strong>{slot.id}</strong> ({slot.type}) - {slot.bfs_distance} steps from entry
          </p>
        </div>
      </div>
      <div className="banner-actions">
        <button className="park-here-btn" onClick={onAccept}>
          <Check size={18} />
          Park Here
        </button>
        <button className="dismiss-btn" onClick={onDismiss}>
          <X size={18} />
        </button>
      </div>
    </div>
  );
};

export default RecommendationBanner;

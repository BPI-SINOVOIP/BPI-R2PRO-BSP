import { TestBed } from '@angular/core/testing';

import { TipsService } from './tips.service';

describe('TipsService', () => {
  let service: TipsService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(TipsService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
